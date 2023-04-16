#include <Arduino.h>
#include <CircularBuffer.h>
#include <DataSource.h>
#include <SD_MMC.h>
#include <run_format.h>

#define INDEX_FILE_PATH "/__INDEX"
#define BLOCK_SIZE 512
#define NUM_BLOCKS 3
#define BLOCK_OVERHEAD 512
// #define DONT_WRITE_HEADER

// constexpr size_t min_block_size(int off = 0) {
//     return off < NUM_SOURCES ? _data_sources[off]._data_size + min_block_size(off + 1) : 0;
// }

template <size_t NUM_SOURCES>
class RunManager {
    typedef run_header_t<NUM_SOURCES> header_t;

    DataSource (&_data_sources)[NUM_SOURCES];
    const uint32_t _cycle_time_base_ms;

    bool _run_is_active;
    File _active_run_file;

    TaskHandle_t _sampler_task;
    TaskHandle_t _writer_task;

    struct block_t {
        SemaphoreHandle_t mutex;
        size_t data_size;
        uint8_t data[BLOCK_SIZE + BLOCK_OVERHEAD];
    } _blocks[NUM_BLOCKS];

   public:
    SemaphoreHandle_t sd_mutex;

    RunManager(uint32_t base_cycle_interval_ms, DataSource (&data_sources)[NUM_SOURCES]) : _data_sources(data_sources),
                                                                                           _cycle_time_base_ms(base_cycle_interval_ms),
                                                                                           sd_mutex(xSemaphoreCreateMutex()) {
        init_blocks();
    }

    header_t generate_header() {
        header_t header = {};
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

    bool init_run() {
        if (!SD_MMC.begin()) {
            return false;
        }

        Serial.printf("SD Initialized! Size: %lumb\n", SD_MMC.cardSize() / (1llu << 20));

        // Set up data sources by setting the cycle time base.
        for (int i = 0; i < NUM_SOURCES; i++)
            _data_sources[i].reset_base_interval(_cycle_time_base_ms);

        return true;
    }

    void start_new_run() {
        finish_current_run();
        init_run();
        _active_run_file = create_run_file();

        header_t header = generate_header();
        Serial.println("Generated new header");

#ifndef DONT_WRITE_HEADER
        write_header(header);
        Serial.println("Wrote header");
#endif

        // Update the index with the new header.
        index_add_new(_active_run_file.name(), header);
        Serial.println("Added header to index");

        _run_is_active = true;

        Serial.println("Creating tasks...");
        xTaskCreate(sampler_task, "Sampler", 4096, this, 15, &_sampler_task);
        xTaskCreate(writer_task, "Writer", 4096, this, 10, &_writer_task);
    }

    File create_run_file() {
        String file_name = get_random_file_name();
        Serial.printf("Beginning new run in %s\n", file_name.c_str());

        // Create run file
        File f = SD_MMC.open(file_name, FILE_WRITE);
        if (!_active_run_file) {
            Serial.println("RUN FILE CREATION PROBLEM!!");
        }

        return f;
    }

    void write_header(header_t &header) {
        if (!_active_run_file)
            return;

        _active_run_file.write((uint8_t *)&header, offsetof(header_t, name));

        _active_run_file.print(header.name);
        _active_run_file.write(0);

        _active_run_file.print(header.description);
        _active_run_file.write(0);

        for (int i = 0; i < NUM_SOURCES; i++) {
            _active_run_file.write((uint8_t *)&(header.entries[i]), sizeof(run_data_source_t));
        }

        // Write the length of the header to file[0]
        uint32_t len = _active_run_file.position();
        _active_run_file.seek(0);
        _active_run_file.write((uint8_t *)&len, sizeof(len));
        _active_run_file.seek(len);
    }

    void finish_current_run() {
        if (_run_is_active) {
            // Note: Find some way to write dredges of data. In worst case
            // currently there will be BLOCK_SIZE bytes of data loss
            vTaskDelete(_sampler_task);
            vTaskDelete(_writer_task);

            _active_run_file.close();

            // Clear blocks
            init_blocks();
            _run_is_active = false;
        }
    }

    void init_blocks() {
        for (size_t i = 0; i < NUM_BLOCKS; i++) {
            _blocks[i].mutex = xSemaphoreCreateMutex();
            _blocks[i].data_size = 0;
        }
    }

    String get_random_file_name() {
        uint8_t rng_buf[16];
        esp_fill_random(rng_buf, sizeof(rng_buf));

        char id_buf[sizeof(rng_buf) * 2 + 2] = {'/'};

        for (size_t i = 0; i < sizeof(rng_buf); i++) {
            sprintf(&(id_buf[i * 2 + 1]), "%02X", rng_buf[i]);
        }

        return String(id_buf);
    }

    void index_add_new(String file_name, run_header_t<NUM_SOURCES> &header) {
        File f = SD_MMC.open(INDEX_FILE_PATH, FILE_APPEND);
        if (!f) {
            Serial.println("!! Error opening index file!");
            return;
        }
        f.printf(R"("%s": {"name":"%s"})", file_name.c_str(), header.name);
        f.println();
        f.close();

        Serial.println("Instantiated index entry!");
    }

    /**
     * Samples data at the interval defined by _cycle_time_base_ms,
     * and writes the data into a block_t to be later handled by the writing
     * task.
     */
    static void sampler_task(void *arg) {
        RunManager *self = (RunManager *)arg;
        TickType_t xLastWakeTime = xTaskGetTickCount();

        size_t block_idx = 0;
        while (1) {
            block_t *current_block = &(self->_blocks)[block_idx++];
            block_idx = block_idx % NUM_BLOCKS;

            if (current_block->data_size != 0) {
                Serial.println("FATAL ERROR: BLOCK OVERRUN (The sampler's next block's size was not equal to zero!)");
                vTaskDelay(portMAX_DELAY);  // Effectively stall the task.
            }

            xSemaphoreTake(current_block->mutex, portMAX_DELAY);
            while (current_block->data_size < BLOCK_SIZE) {
                for (int i = 0; i < NUM_SOURCES; i++) {
                    current_block->data_size += self->_data_sources[i].cycle(&(current_block->data)[current_block->data_size]);
                }
                vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(self->_cycle_time_base_ms));
                // Serial.println(current_block->data_size);
            }
            xSemaphoreGive(current_block->mutex);
        }
    }

    /**
     * Waits for data in a block_t and writes it to disk.
     * Lower priority than the sampler, which (hopefully)
     * prevents SD accesses from blocking sampling.
     */
    static void writer_task(void *arg) {
        RunManager *self = (RunManager *)arg;
        TickType_t xLastWakeTime = xTaskGetTickCount();

        size_t block_idx = 0;
        while (1) {
            // Select a block cyclically. (IE 0 -> 1 -> 2 -> 0 -> 1 ...)
            block_t *current_block = &(self->_blocks)[block_idx++];
            block_idx = block_idx % NUM_BLOCKS;

            // Wait on the sampler to return the target block's mutex.
            // Once this is taken we know that the sampler has
            // filled the block, ready for writing.
            xSemaphoreTake(current_block->mutex, portMAX_DELAY);
            if (current_block->data_size == 0) {
                Serial.println("FATAL ERROR: ZERO-SIZE BLOCK ENCOUNTERED IN WRITER! (this signifies a desync between the sampler and writer. Maybe the sampler crashed?)");
                vTaskDelay(portMAX_DELAY);  // Effectively stall the task.
            }

            // Take the SD mutex to gain exclusive rights to write to the SD card.
            xSemaphoreTake(self->sd_mutex, portMAX_DELAY);

            self->_active_run_file.write(current_block->data, current_block->data_size);
            current_block->data_size = 0;

            // Return mutexes to allow other things to access resources.
            xSemaphoreGive(self->sd_mutex);
            xSemaphoreGive(current_block->mutex);

            Serial.println("Wrote block!");
        }
    }

    void index_delete() {
    }
};