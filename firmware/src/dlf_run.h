#pragma once

#include <Arduino.h>
#include <FS.h>
#include <stdint.h>
#include <uuid.h>

#include <chrono>
#include <memory>
#include <vector>

#include "datastream/AbstractStream.hpp"
#include "dlf_cfg.h"
#include "dlf_logfile.hpp"
#include "dlf_types.h"

namespace dlf {
using namespace std;

class Run {
    String _uuid;
    FS &_fs;
    dlf_file_state_e _status;
    SemaphoreHandle_t _sync;
    chrono::microseconds _tick_interval;
    streams_t _streams;
    vector<unique_ptr<LogFile>> _log_files;

   public:
    Run(FS &fs) : _fs(fs) {
        _uuid = "/" + StringUUIDGen();
        _sync = xSemaphoreCreateCounting(1, 0);
    }

    template <typename T>
    bool begin(streams_t all_streams, chrono::microseconds tick_interval, T &meta) {
        // Check tick interval isn't 0
        if (tick_interval.count() <= 0) return false;

        Serial.printf("Starting run %s", _uuid.c_str());

        _streams = all_streams;
        _tick_interval = tick_interval;

        // Make directory to contain run files.
        _fs.mkdir(_uuid);

        // Write meta
        File f = _fs.open(_uuid + "/meta", "w", true);
        f.write(reinterpret_cast<uint8_t *>(&meta), sizeof(T));
        f.close();

        // Create logfile instances
        create_logfile_for(POLLED);
        create_logfile_for(EVENT);

        Serial.println("Logfiles inited");

        _status = LOGGING;

        // Setup ticks
        xTaskCreate(task_sampler, "Sampler", 4096, this, 5, NULL);

        return true;
    }

    void create_logfile_for(dlf_stream_type_e t) {
        stream_handles_t handles;

        for (size_t i = 0, e = _streams.size(); i != e; ++i) {
            AbstractStream *ds = _streams[i];

            if (ds->type() == t) {
                handles.push_back(move(ds->handle(_tick_interval, i)));
            }
        }
        _log_files.push_back(unique_ptr<LogFile>(new LogFile(move(handles), t, _uuid, _fs)));
    }

    /**
     * NOTE: Has caused canary issues if stack too small (1024 is problematic). 
     * @brief 
     * @param arg 
     */
    static void task_sampler(void *arg) {
        Serial.println("Sampler inited");
        auto self = static_cast<Run *>(arg);

        const TickType_t interval = chrono::duration_cast<DLF_FREERTOS_DURATION>(self->_tick_interval).count();
        Serial.printf("Interval %d\n", interval);

        TickType_t prev_run;

        // Run at constant tick interval
        for (dlf_tick_t tick = 0; self->_status == LOGGING; tick++) {
            Serial.println("SAMPLING");

            for (auto &lf : self->_log_files) {
                lf->sample(tick);
            }
            xTaskDelayUntil(&prev_run, interval);
        }
        Serial.println("Sampler exiting!");
        xSemaphoreGive(self->_sync);
        vTaskDelete(NULL);
    }

    void close() {
        Serial.println("Closing run!");
        _status = FLUSHING;

        // Wait for sampling task to cleanly exit.
        xSemaphoreTake(_sync, portMAX_DELAY);
        vSemaphoreDelete(_sync);

        for (auto &lf : _log_files) {
            lf->close();
        }

        Serial.println("Run closed cleanly!");
    }
};

}  // namespace dlf