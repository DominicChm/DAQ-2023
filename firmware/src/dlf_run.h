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
    chrono::microseconds tick_interval;
    streams_t _streams;
    vector<unique_ptr<LogFile>> log_files;

   public:
    Run(FS &fs) : _fs(fs) {
        _uuid = StringUUIDGen();
        _sync = xSemaphoreCreateCounting(1, 0);
    }

    template <typename T>
    bool begin(streams_t all_streams, chrono::microseconds tick_interval, T &meta) {
        _streams = all_streams;

        _fs.mkdir(_uuid);

        // Write meta
        File f = _fs.open(_uuid + "/meta", "w", true);
        f.write(reinterpret_cast<uint8_t *>(&meta), sizeof(T));
        f.close();
        
        // Create logfile instances
        create_logfile_for(DLF_POLLED);
        create_logfile_for(DLF_EVENT);

        // Setup ticks

        xTaskCreate(task_sampler());

        return true;
    }

    void create_logfile_for(DLFStreamType t) {
        stream_handles_t handles;

        for (size_t i = 0, e = _streams.size(); i != e; ++i) {
            AbstractStream *ds = _streams[i];

            if (ds->type() == t) {
                handles.push_back(move(ds->handle(tick_interval, i)));
            }
        }
        log_files.push_back(unique_ptr<LogFile>(new LogFile(move(handles), DLF_POLLED, _uuid, _fs)));
    }

    void task_sampler(void *arg) {
        auto self = static_cast<Run *>(arg);

        const TickType_t interval = chrono::duration_cast<DLF_FREERTOS_DURATION>(tick_interval).count();
        TickType_t prev_run;

        // Run at constant tick interval
        for (dlf_tick_t tick = 0; self->_status == LOGGING; tick++) {
            for (auto &lf : log_files) {
                lf->sample(tick);
            }
            xTaskDelayUntil(&prev_run, interval);
        }

        xSemaphoreGive(self->_sync);
        vTaskDelete(NULL);
    }

    void close() {
        // Wait for sampling task to cleanly exit.
        xSemaphoreTake(_sync, portMAX_DELAY);
        vSemaphoreDelete(_sync);

        for (auto &lf : log_files) {
            lf->close();
        }
    }
};

}  // namespace dlf