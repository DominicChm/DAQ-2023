#include <run_format.h>
#include <DataSource.h>
#include <SD_MMC.h>
#include <CircularBuffer.h>
#include <Arduino.h>

#define INDEX_FILE_PATH "/__INDEX"
#define BLOCK_SIZE 64
#define NUM_BLOCKS 3
#define BLOCK_OVERHEAD BLOCK_SIZE

#define DONT_WRITE_HEADER

template <size_t NUM_SOURCES>
class RunManager
{
    DataSource (&data_sources)[NUM_SOURCES];
    const uint32_t base_cycle_interval_ms;

    bool run_is_active;
    File active_run_file;

    TaskHandle_t sampler_task;
    TaskHandle_t writer_task;

    struct block_t
    {
        SemaphoreHandle_t mutex;
        size_t data_size;
        uint8_t data[BLOCK_SIZE + BLOCK_OVERHEAD];
    } blocks[NUM_BLOCKS];

public:
    RunManager(uint32_t base_cycle_interval_ms, DataSource (&data_sources)[NUM_SOURCES]) : data_sources(data_sources),
                                                                                           base_cycle_interval_ms(base_cycle_interval_ms)
    {
    }
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
        init();
        finish_current_run();

        String file_name = get_random_file_name();
        Serial.printf("Beginning new run in %s\n", file_name.c_str());

        // Create run file
        active_run_file = SD_MMC.open(file_name, FILE_WRITE);
        if (!active_run_file)
        {
            Serial.println("RUN FILE CREATION PROBLEM!!");
            return;
        }

        // Set up the header
        run_header_t<NUM_SOURCES> head = {0};
        head.base_cycle_interval_ms = base_cycle_interval_ms;
        head.start_time_epoch = 0;
        head.location = run_location_t{
            .longitude = .6,
            .latitude = .5,
        };
        head.num_sources = NUM_SOURCES;

        // Update the index with the new header.
        index_add_new(file_name, head);

        // Set up data sources by setting the cycle time base.
        for (int i = 0; i < NUM_SOURCES; i++)
            data_sources[i].reset_base_interval(base_cycle_interval_ms);

        // init blocks
        for (size_t i = 0; i < NUM_BLOCKS; i++)
        {
            blocks[i].mutex = xSemaphoreCreateMutex();
            blocks[i].data_size = 0;
        }
        run_is_active = true;

        Serial.println("Creating tasks...");
        xTaskCreate(sampler, "Sampler", 4096, this, 15, &sampler_task);
        xTaskCreate(writer, "Writer", 4096, this, 10, &writer_task);
    }

    void finish_current_run()
    {
        if (run_is_active)
        {
            // Note: Find some way to write dredges of data. In worst case
            // currently there will be 512 bytes of data loss
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

        size_t block_idx = 0;
        while (1)
        {
            block_t *current_block = &(self->blocks)[block_idx++];
            block_idx = block_idx % NUM_BLOCKS;

            if (current_block->data_size != 0)
            {
                Serial.println("FATAL ERROR: BLOCK OVERRUN (The sampler's next block's size was not equal to zero!)");
                vTaskDelay(portMAX_DELAY); // Effectively stall the task.
            }

            xSemaphoreTake(current_block->mutex, portMAX_DELAY);
            while (current_block->data_size < BLOCK_SIZE)
            {
                for (int i = 0; i < NUM_SOURCES; i++)
                {
                    current_block->data_size += self->data_sources[i].cycle(&(current_block->data)[current_block->data_size]);
                }
                vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(self->base_cycle_interval_ms));
                Serial.println(current_block->data_size);
            }
            xSemaphoreGive(current_block->mutex);
        }
    }

    static void writer(void *arg)
    {
        RunManager *self = (RunManager *)arg;
        TickType_t xLastWakeTime = xTaskGetTickCount();

        size_t block_idx = 0;
        while (1)
        {
            block_t *current_block = &(self->blocks)[block_idx++];
            block_idx = block_idx % NUM_BLOCKS;

            xSemaphoreTake(current_block->mutex, portMAX_DELAY);
            self->active_run_file.write(current_block->data, current_block->data_size);
            current_block->data_size = 0;
            xSemaphoreGive(current_block->mutex);

            Serial.println("Wrote block!");
        }
    }
    void index_delete()
    {
    }
};