#include <run_format.h>
#include <DataSource.h>
#include <SD_MMC.h>
#include <CircularBuffer.h>
#include <Arduino.h>

#define INDEX_FILE_PATH "/__INDEX"

template <size_t NUM_SOURCES>
class RunManager
{
    const DataSource (&data_sources)[NUM_SOURCES];
    const uint32_t base_cycle_interval_ms;

    bool run_is_active;
    File active_run_file;
    TaskHandle_t sampler_task;
    TaskHandle_t writer_task;

    SemaphoreHandle_t sd_mutex;
    SemaphoreHandle_t sample_mutex;

    CircularBuffer<uint8_t, 1024 * 8> sample_buf;

public:
    RunManager(uint32_t base_cycle_interval_ms, const DataSource (&data_sources)[NUM_SOURCES]) : data_sources(data_sources),
                                                                                                 base_cycle_interval_ms(base_cycle_interval_ms) {}
    bool init()
    {
        if (!SD_MMC.begin())
        {
            return false;
        }

        Serial.printf("SD Initialized! Size: %lumb\n", SD_MMC.cardSize() / (1llu << 20));
        return true;
    }

    void start_new_run()
    {
        finish_current_run();

        String file_name = get_random_file_name();
        Serial.printf("Beginning new run in %s\n", file_name.c_str());

        // Create run file
        active_run_file = SD_MMC.open(file_name, FILE_WRITE);
        run_is_active = true;

        run_header_t<NUM_SOURCES> head = {0};
        head.base_cycle_interval_ms = base_cycle_interval_ms;
        head.start_time_epoch = 0;
        head.location = run_location_t{
            .longitude = .6,
            .latitude = .5,
        };
        head.num_sources = NUM_SOURCES;
        index_add_new(file_name, head);

        sample_buf.clear();
        xTaskCreate(sampler, "Sampler", 4096, this, 10, &sampler_task);
        xTaskCreate(writer, "Sampler", 4096, this, 10, &sampler_task);

        // Instantiate a run on that file
    }

    void finish_current_run()
    {
        if (run_is_active)
        {
            vTaskDelete(sampler_task);
            vTaskDelete(writer_task);

            active_run_file.close();
            run_is_active = false;
        }
    }

    String get_random_file_name()
    {
        uint8_t rng_buf[16];
        esp_fill_random(rng_buf, sizeof(rng_buf));

        char id_buf[sizeof(rng_buf) * 2 + 2] = {'/'};

        for (size_t i = 0; i < sizeof(rng_buf); i++)
        {
            sprintf(&(id_buf[i * 2 + 1]), "%02X", rng_buf[i]);
        }

        return String(id_buf);
    }

    void index_add_new(String file_name, run_header_t<NUM_SOURCES> &header)
    {
        File f = SD_MMC.open(INDEX_FILE_PATH, FILE_APPEND);
        if (!f)
        {
            Serial.println("!! Error opening index file!");
            return;
        }
        f.printf(R"("%s": {"name":"%s"})", file_name.c_str(), header.name);
        f.println();
        f.close();

        Serial.println("Instantiated index entry!");
    }

    static void sampler(void *arg)
    {
        RunManager *self = (RunManager *)arg;
        TickType_t xLastWakeTime = xTaskGetTickCount();
        uint8_t block[1024];
        while (1)
        {
            xSemaphoreTake(self->sample_mutex, 1000);
            // Sample all sensors into a local buffer, then
            // write to the circular buffer.
            for (DataSource d : data_sources)
            {
                int len = d.cycle(&block);
                for (int i = 0; i < len; i++)
                    sample_buf.push(block[i]);
            }

            // Push said local buffer into the circular buffer
            for (size_t i = 0; i < idx; i++)

                xSemaphoreGive(self->sample_mutex);
            vTaskDelayUntil(&xLastWakeTime, self->base_cycle_interval_ms);
        }
    }

    static void writer(void *arg)
    {
        RunManager *self = (RunManager *)arg;
        TickType_t xLastWakeTime = xTaskGetTickCount();

        uint8_t block[512];
        while (1)
        {
            vTaskDelayUntil(&xLastWakeTime, 10);

            xSemaphoreTake(self->sample_mutex, 1000);
            if (self->sample_buf.size() < sizeof(block))
            {
                xSemaphoreGive(self->sample_mutex);
                continue;
            }
            // Read 512 bytes from the circular buffer into a local block of memory to cache for the SD write.
            for (int i = 0; i < sizeof(block); i++)
                block[i] = self->sample_buf.shift();

            xSemaphoreGive(self->sample_mutex);

            // SD WRITE
            xSemaphoreTake(self->sd_mutex, 1000);
            self->active_run_file.write(&block, sizeof(block));
            xSemaphoreGive(self->sd_mutex);
        }
    }
    void index_delete()
    {
    }
};