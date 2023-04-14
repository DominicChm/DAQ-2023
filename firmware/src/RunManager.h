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
    typedef run_header_t<NUM_SOURCES> header_t;

    DataSource (&_data_sources)[NUM_SOURCES];
    const uint32_t _cycle_time_base_ms;

    bool _run_is_active;
    File _active_run_file;

    TaskHandle_t _sampler_task;
    TaskHandle_t _writer_task;

    struct block_t
    {
        SemaphoreHandle_t mutex;
        size_t data_size;
        uint8_t data[BLOCK_SIZE + BLOCK_OVERHEAD];
    } _blocks[NUM_BLOCKS];

public:
    RunManager(uint32_t base_cycle_interval_ms, DataSource (&data_sources)[NUM_SOURCES]) : _data_sources(data_sources),
                                                                                           _cycle_time_base_ms(base_cycle_interval_ms)
    {
    }

    header_t generate_header()
    {
        header_t header;
        header.cycle_time_base_ms = _cycle_time_base_ms;

        // Set up header sources
        for (int i = 0; i < NUM_SOURCES; i++)
            header.entries[i] = _data_sources[i].header_entry();

        // Metadata
        header.set_name("NAME");
        header.set_description("DESC");

        // FIXME: Update these when data available!!
        // Plan is to use GPS data to provide these.
        header.start_time_epoch = 0;
        header.location = run_location_t{
            .longitude = .6,
            .latitude = .5,
        };

        return header;
    }

    bool init_run()
    {
        if (!SD_MMC.begin())
        {
            return false;
        }

        Serial.printf("SD Initialized! Size: %lumb\n", SD_MMC.cardSize() / (1llu << 20));

        // Set up data sources by setting the cycle time base.
        for (int i = 0; i < NUM_SOURCES; i++)
            _data_sources[i].reset_base_interval(_cycle_time_base_ms);

        return true;
    }

    void start_new_run()
    {
        finish_current_run();
        init_run();

        String file_name = get_random_file_name();
        Serial.printf("Beginning new run in %s\n", file_name.c_str());

        // Create run file
        _active_run_file = SD_MMC.open(file_name, FILE_WRITE);
        if (!_active_run_file)
        {
            Serial.println("RUN FILE CREATION PROBLEM!!");
            return;
        }

        header_t header = generate_header();

        // Update the index with the new header.
        index_add_new(file_name, header);

        _run_is_active = true;

        Serial.println("Creating tasks...");
        xTaskCreate(sampler, "Sampler", 4096, this, 15, &_sampler_task);
        xTaskCreate(writer, "Writer", 4096, this, 10, &_writer_task);
    }

    void write_header(header_t header)
    {
        if (!_active_run_file)
            return;
        _active_run_file.write(&header, offsetof(header_t, name));
    }

    void finish_current_run()
    {
        if (_run_is_active)
        {
            // Note: Find some way to write dredges of data. In worst case
            // currently there will be BLOCK_SIZE bytes of data loss
            vTaskDelete(_sampler_task);
            vTaskDelete(_writer_task);

            _active_run_file.close();

            // Clear blocks
            for (size_t i = 0; i < NUM_BLOCKS; i++)
            {
                _blocks[i].mutex = xSemaphoreCreateMutex();
                _blocks[i].data_size = 0;
            }

            _run_is_active = false;
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
            block_t *current_block = &(self->_blocks)[block_idx++];
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
                    current_block->data_size += self->_data_sources[i].cycle(&(current_block->data)[current_block->data_size]);
                }
                vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(self->_cycle_time_base_ms));
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
            block_t *current_block = &(self->_blocks)[block_idx++];
            block_idx = block_idx % NUM_BLOCKS;

            xSemaphoreTake(current_block->mutex, portMAX_DELAY);
            self->_active_run_file.write(current_block->data, current_block->data_size);
            current_block->data_size = 0;
            xSemaphoreGive(current_block->mutex);

            Serial.println("Wrote block!");
        }
    }
    void index_delete()
    {
    }
};