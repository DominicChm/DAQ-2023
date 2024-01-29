#include <Arduino.h>
#include <FS.h>
#include <stdint.h>
#include <uuid.h>

#include <chrono>
#include <vector>

#include "datastream/AbstractStream.hpp"
#include "dlf_cfg.h"
#include "dlf_types.h"
#include "dlf_logfile.hpp"

namespace dlf {
using namespace std;

class Run {
    String _uuid;
    FS &_fs;
    QueueHandle_t _sample_q = xQueueCreate(DLF_QUEUE_SIZE, sizeof(dlf_sample_queue_entry));
    dlf_file_state_e _status;
    SemaphoreHandle_t _sync;
    chrono::microseconds tick_interval;

    vector<LogFile> log_files;

public:
    Run(streams_t all_streams, chrono::microseconds tick_interval, FS &fs) : _fs(fs) {
        _uuid = StringUUIDGen();
        _sync = xSemaphoreCreateCounting(2, 2);

        log_files.push_back(LogFile(all_streams, DLF_POLLED, tick_interval, _uuid, fs, _sample_q));
        log_files.push_back(LogFile(all_streams, DLF_EVENT, tick_interval, _uuid, fs, _sample_q));

    }

    template <typename T>
    bool begin(T &meta) {
        // Setup ticks
        _fs.mkdir(_uuid);

        // Write meta  
        File f = _fs.open(_uuid + "/meta", "w", true);
        f.write(reinterpret_cast<uint8_t*>(&meta), sizeof(T));
        f.close();

        
        return true;
    }

    void sample() {
        // for each logfile run sample
    }

    /**
     * Waits on data chunks to arrive in the sample queue from dlf_logfile classes.
     * When they arrive, this task simply writes them to the specified file on SD.
     * @brief
     */
    void write_samples() {
        dlf_sample_queue_entry s;
        BaseType_t res = xQueueReceive(_sample_q, &s, pdMS_TO_TICKS(100));
        if (res != pdTRUE) return;

        // Write to SD
        if (!s.f) return;

        s.f->write(s.buf->data(), s.buf->size());

        // Dealloc vector
        delete s.buf;
    }

    void task_writer(void *arg) {
        auto self = static_cast<Run *>(arg);
        xSemaphoreTake(self->_sync, portMAX_DELAY);

        while (self->_status == LOGGING || uxQueueMessagesWaiting(self->_sample_q) > 0) {
            self->write_samples();
        }

        xSemaphoreGive(self->_sync);
        vTaskDelete(NULL);
    }

    void task_sampler(void *arg) {
        auto self = static_cast<Run *>(arg);
        xSemaphoreTake(self->_sync, portMAX_DELAY);

        const TickType_t interval = chrono::duration_cast<DLF_FREERTOS_DURATION>(tick_interval).count();
        TickType_t prev_run;

        // Run at constant tick interval
        while (self->_status == LOGGING) {
            self->sample();
            xTaskDelayUntil(&prev_run, interval);
        }

        xSemaphoreGive(self->_sync);
        vTaskDelete(NULL);
    }

    void finish() {
        // Wait for both writing and sampling task to cleanly exit.
        xSemaphoreTake(_sync, portMAX_DELAY);
        xSemaphoreTake(_sync, portMAX_DELAY);

        // Finalize all logfiles
    }
};

}  // namespace dlf