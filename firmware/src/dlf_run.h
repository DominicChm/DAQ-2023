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


// TODO: Tick timings

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
        assert(tick_interval.count() > 0);

        Serial.printf("Starting run %s\n", _uuid.c_str());

        _streams = all_streams;
        _tick_interval = tick_interval;

        // Make directory to contain run files.
        _fs.mkdir(_uuid);

        // Write meta
        File f = _fs.open(_uuid + "/meta", "w", true);
        f.write(reinterpret_cast<uint8_t *>(&meta), sizeof(T));
        f.close();

        // Writes metafile for this log.
        create_metafile();

        // Create logfile instances
        create_logfile_for(POLLED);
        create_logfile_for(EVENT);

        Serial.println("Logfiles inited");

        _status = LOGGING;

        // Setup ticks
        xTaskCreate(task_sampler, "Sampler", 4096 * 2, this, 5, NULL);

        return true;
    }

    void create_metafile() {

    }

    void create_logfile_for(dlf_stream_type_e t) {
        stream_handles_t handles;
#ifdef DEBUG
        DEBUG.printf("Creating %s logfile\n", stream_type_to_string(t));
#endif
        size_t idx = 0;
        for (auto &stream : _streams) {
            if (stream->type() == t) {
                handles.push_back(move(stream->handle(_tick_interval, idx++)));
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
#ifdef DEBUG
            DEBUG.printf("Sample\n\ttick: %d\n", tick);
#endif
            for (auto &lf : self->_log_files) {
                lf->sample(tick);
            }
            xTaskDelayUntil(&prev_run, interval);
        }
#ifdef DEBUG
        DEBUG.println("Sampler exiting cleanly");
#endif

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