#pragma once
#include <freertos/stream_buffer.h>

#include <vector>

#include "FS.h"
#include "datastream/AbstractStreamHandle.hpp"
#include "datastream/EventStream.hpp"
#include "datastream/PolledStream.hpp"
#include "dlf_cfg.h"
#include "dlf_types.h"
#include "uuid.h"

namespace dlf {
using namespace datastream;
using namespace std;

/**
 * @brief Handles logging of datastreams to files.
 *
 * Handles a SINGLE TYPE of datastream handle (IE polled/event).
 * All data sampled by a LogFile class is written into a single contiguous file.
 * Sampled data is written into chunks that are then pushed to the provided queue.
 * The provided queue should write the passed chunks to the specified pointer.
 *
 * LogFiles do NOT create tasks themselves. All RTOS task creation is handled by
 * the Run class (which manages multiple LogFile instances)
 *
 * https://stackoverflow.com/questions/8915873/how-much-work-should-constructor-of-my-class-perform
 */
class LogFile {
   protected:
    stream_handles_t _handles;

    FS &_fs;
    String _filename;
    File _f;

    StreamBufferHandle_t _stream;
    dlf_file_state_e _state;
    SemaphoreHandle_t _sync;
    dlf_tick_t _last_tick;

    /**
     * @brief Writes a complete header into this logfile.
     *
     * Uses existing streambuffer architecture because why not.
     */
    void _write_header(dlf_stream_type_e stream_type) {
        dlf_logfile_header_t h;
        h.stream_type = stream_type;
        h.num_streams = _handles.size();
        xStreamBufferSend(_stream, &h, sizeof(h), portMAX_DELAY);

        for (auto &handle : _handles) {
            handle->encode_header_into(_stream);
        }
    }

   public:
    LogFile(stream_handles_t handles, dlf_stream_type_e stream_type, String dir, FS &fs)
        : _fs(fs), _handles(std::move(handles)) {
        _filename = dir + "/" + stream_type_to_string(stream_type) + ".dlf";

        // Set up class internals
        _stream = xStreamBufferCreate(DLF_LOGFILE_BUFFER_SIZE, DLF_SD_BLOCK_WRITE_SIZE);
        if (_stream == NULL) {
            _state = STREAM_CREATE_ERROR;
            return;
        }

        _sync = xSemaphoreCreateCounting(1, 0);
        if (_sync == NULL) {
            _state = SYNC_CREATE_ERROR;
            return;
        }

        // Open logfile
        _f = _fs.open(_filename, "w", true);

        if (!_f) {
            _state = FILE_OPEN_ERROR;
            return;
        }

        // Init data flusher
        _state = LOGGING;

        if (xTaskCreate(task_flusher, "Flusher",  2048, this, 5, NULL) != pdTRUE) {
            _state = FLUSHER_CREATE_ERROR;
            return;
        }

        // Initialize logfile
        _write_header(stream_type);
    }

    /**
     * @brief Samples data. Intended to be externally called at the tick interval.
     * @param tick
     */
    void sample(dlf_tick_t tick) {
        if (_state != LOGGING) return;

        _last_tick = tick;

        // Sample all handles
        for (auto &h : _handles) {
            if (h->available(tick)) {
                h->encode_into(_stream, tick);
                if (xStreamBufferIsFull(_stream)) _state = QUEUE_FULL;
            }
        }
    }

    static void task_flusher(void *arg) {
        auto self = static_cast<LogFile *>(arg);

        uint8_t buf[DLF_SD_BLOCK_WRITE_SIZE];

        while (self->_state == LOGGING) {
            size_t received = xStreamBufferReceive(self->_stream, buf, sizeof(buf), pdMS_TO_TICKS(1000));
            self->_f.write(buf, received);
        }

        // If errored, exit immediately
        if (self->_state != FLUSHING) {
            Serial.printf("FLUSHER ERROR WITH %x\n", self->_state);

            vTaskDelete(NULL);
            return;
        }

        Serial.println("Flushing remaining bytes...");
        // Flush remaining bytes
        while (xStreamBufferBytesAvailable(self->_stream) > 0 && self->_state == FLUSHING) {
            size_t received = xStreamBufferReceive(self->_stream, buf, sizeof(buf), 0);
            self->_f.write(buf, received);
        }

        self->_state == FLUSHED;
        xSemaphoreGive(self->_sync);

        Serial.printf("Flusher exited cleanly w/ HWM %u\n", uxTaskGetStackHighWaterMark(NULL));
        vTaskDelete(NULL);
    }

    void close() {
        if (_state != LOGGING) return;

        _state = FLUSHING;
        Serial.println("Closing logfile");
        xSemaphoreTake(_sync, portMAX_DELAY);  // wait for flusher to finish up.
        Serial.println("Continue logfile close");

        // Cleanup dynamic allocations
        vStreamBufferDelete(_stream); // FIXME: this crashes for some reason.
        vSemaphoreDelete(_sync);
        Serial.println("sem delteted");



        // Update header with # of ticks
        _f.seek(offsetof(dlf_logfile_header_t, tick_span));
        _f.write(reinterpret_cast<uint8_t *>(&_last_tick), sizeof(dlf_tick_t));
        Serial.println("calling close");

        _f.close();
        Serial.println("Logfile closed cleanly");
    }
};
}  // namespace dlf