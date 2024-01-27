#include <Arduino.h>
#include <SD_MMC.h>

#include "dlf_datastream.h"
#include "dlf_logfile.hpp"
#include "dlf_types.h"

#define INDEX_FILE_PATH "/__INDEX"
#define BLOCK_SIZE 512
#define NUM_BLOCKS 10
#define BLOCK_OVERHEAD 512
// #define DONT_WRITE_HEADER

class DLFLogger {
    std::vector<DLFLogFile> log_files;
    std::vector<DLFDataStream *> data_streams;

    FS &fs;
    String fs_dir;

   public:
    DLFLogger(FS &fs, String fs_dir = "/") : fs(fs), fs_dir(fs_dir) {
    }

    bool init_run() {
        if (!SD_MMC.begin()) {
            Serial.println("No SD card detected!");
            return false;
        }

        Serial.printf("SD Initialized! Size: %lumb\n", SD_MMC.cardSize() / (1llu << 20));

        total_cycles_stored = 0;

        return true;
    }

    void start_new_run() {
        uint32_t t_start = millis();

        finish_current_run();
        if (!init_run()) return;

        Serial.printf("Starting logging with a cycle time-base of %dms\n", _cycle_time_base_ms);

        // Update the index with the new header.
        index_add_new(_active_run_file.name(), header);
        Serial.println("Added header to index");

        _run_is_active = true;

        Serial.println("Creating tasks...");
        xTaskCreate(sampler_task, "Sampler", 4096, this, 15, &_sampler_task);
        xTaskCreate(writer_task, "Writer", 4096, this, 10, &_writer_task);
        Serial.printf("Logging started in %ums! Send \"stop\" to finish.\n", millis() - t_start);
    }

    File create_run_file() {
        String file_name = get_random_file_name();
        Serial.printf("Beginning new run in %s\n", file_name.c_str());

        // Create run file
        File f = SD_MMC.open(file_name, FILE_WRITE);
        if (!f) {
            Serial.println("RUN FILE CREATION PROBLEM!! RESTARTING.");
            ESP.restart();
        }

        return f;
    }

    void write_header() {
        if (!_active_run_file)
            return;

        _active_run_file.write((uint8_t *)&header, sizeof(header_t));
    }

    void finish_current_run() {
        if (_run_is_active) {
            // Note: Find some way to write dredges of data. In worst case
            // currently there will be BLOCK_SIZE bytes of data loss
            vTaskDelete(_sampler_task);
            vTaskDelete(_writer_task);

            _active_run_file.seek(offsetof(header_t, num_cycles));
            _active_run_file.write(total_cycles_stored);
            _active_run_file.close();

            // Clear blocks
            init_blocks();
            _run_is_active = false;

            Serial.println("Existing run finished and closed.");
        }
    }

    void init_blocks() {
        for (size_t i = 0; i < NUM_BLOCKS; i++) {
            _blocks[i].mutex = xSemaphoreCreateMutex();
            _blocks[i].data_size = 0;
            _blocks[i].cycles_stored = 0;
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

    void index_add_new(String file_name, run_header_t<MAX_NUM_SOURCES> &header) {
        File f = SD_MMC.open(INDEX_FILE_PATH, FILE_APPEND);
        if (!f) {
            Serial.println("!! Error opening index file!");
            return;
        }
        f.printf(R"("%s": {
            "name":"%s", 
            "description": "%s",
            "time_base": %u,
            "start_time": %u,
            "location": {"longitude": %f, "latitude": %f}
        },)",
                 file_name.c_str(),
                 header.name,
                 header.description,
                 header.cycle_time_base_ms,
                 header.start_time_epoch,
                 header.location.longitude, header.location.latitude);
        f.println();
        f.close();

        Serial.println("Instantiated index entry!");
    }

    constexpr size_t header_size() {
        return sizeof(header_t);
    }

    void write_current_entries() {
        File f = SD_MMC.open("/__ENTRIES", FILE_WRITE);
        update_header_entries();
        f.write((uint8_t *)&header.num_entries, sizeof(header.num_entries) + sizeof(header.entries));
        f.close();
    }

    void index_delete() {
    }

    template <typename T>
    DLFDataStream &log(T &dat, std::chrono::duration<dlf_tick_t, std::milli> sample_interval, const char *id, const char *type_id = characteristic_type_name<T>()) {
        DLFDataStream d;
    }
};