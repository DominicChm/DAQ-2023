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

// Todo: Performance timings

class Run {
    String _uuid;
    FS &_fs;
    dlf_file_state_e _status;
    SemaphoreHandle_t _sync;
    chrono::microseconds _tick_interval;
    streams_t _streams;
    vector<unique_ptr<LogFile>> _log_files;

   public:
    template <typename T>
    Run(FS &fs, streams_t all_streams, chrono::microseconds tick_interval, T &meta)
        : _fs(fs), _streams(all_streams), _tick_interval(tick_interval) {
        assert(tick_interval.count() > 0);

        _uuid = "/" + StringUUIDGen();
        _sync = xSemaphoreCreateCounting(1, 0);

#ifdef DEBUG
        Serial.printf("Starting run %s\n", _uuid.c_str());
#endif

        // Make directory to contain run files.
        _fs.mkdir(_uuid);

        // Writes metafile for this log.
        create_metafile(meta);

        // Create logfile instances
        create_logfile_for(POLLED);
        create_logfile_for(EVENT);

        Serial.println("Logfiles inited");

        _status = LOGGING;

        // Setup ticks
        xTaskCreate(task_sampler, "Sampler", 4096 * 2, this, 5, NULL);
    }

    template <typename T>
    void create_metafile(T &meta) {
#ifdef DEBUG
        DEBUG.print("Creating metafile... ");
#endif
        File f = _fs.open(_uuid + "/meta.dlf", "w", true);

        dlf_meta_header_t h;
        h.tick_base_us = _tick_interval.count();
        strlcpy(h.application, "TESTAPP", sizeof(h.application));
        h.meta_size = sizeof(T);

        f.write(reinterpret_cast<uint8_t *>(&h), sizeof(h));
        f.write(reinterpret_cast<uint8_t *>(&meta), sizeof(T));
        f.close();
#ifdef DEBUG
        DEBUG.print("Done!\n");
#endif
    }

    void create_logfile_for(dlf_stream_type_e t) {
#ifdef DEBUG
        DEBUG.printf("Creating %s logfile", stream_type_to_string(t));
#endif
        stream_handles_t handles;

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
#if defined(DEBUG) && defined(SILLY)
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