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
 */
class LogFile {
   protected:
    stream_handles_t _handles;

    FS &_fs;
    String _filename;
    File _f;

    /**
     * @brief Handle to an external queue. dlf_sample_queue_entrys passed into this
     * queue should be written to FS.
     */
    QueueHandle_t _writer_queue;

    bool is_logging = true;
    dlf_file_state_e _state = UNINITIALIZED;

    /**
     * @brief Ptr to heap-allocated sample buffer. When filled, the ptr to
     * this buffer is handed off to the _writer_queue for writing to disk.
     */
    vector<uint8_t> *_sample_buf = new_sample_buf();

   public:
    LogFile(streams_t all_streams, DLFStreamType stream_type, microseconds tick_interval, String dir, FS &fs, QueueHandle_t writer_queue)
        : _fs(fs), _writer_queue(writer_queue) {
        _handles = create_stream_handles(all_streams, tick_interval, stream_type);
        _filename = dir + "/" + stream_type_to_string(stream_type);
    }

    static vector<uint8_t> *new_sample_buf() {
        return new vector<uint8_t>(DLF_MIN_SAMPLE_BUFFER * 2);
    }

    /**
     * @brief Samples data. Intended to be externally called at the tick interval.
     * @param tick 
     */
    void sample(dlf_tick_t tick) {
        if (_state != LOGGING) return;

        if (_sample_buf == nullptr) {
            Serial.println("Out of heap mem :(");
            _state = HEAP_FULL;
            return;
        }

        // Sample all handles into current sample buffer.
        for (auto &h : _handles) {
            if (h->available(tick))
                h->encode_into(*_sample_buf, tick);
        }

        // If buffer full, hand off ptr to write queue
        if (_sample_buf->size() > DLF_MIN_SAMPLE_BUFFER) {
            dlf_sample_queue_entry e{&_f, _sample_buf};
            BaseType_t res = xQueueSendToBack(_writer_queue, _sample_buf, 0);

            if (res != pdTRUE) {
                // Data is read faster than the writer could go.
                Serial.println("No space in sampling queue!");
                _state = QUEUE_FULL;
                return;
            }

            // Init a new sample buf to use next tick
            _sample_buf = new_sample_buf();
        }
    }

    void write_header() {
        for (auto &handle : _handles) {
            // ... Todo
        }
    }

    bool begin() {
        _f = _fs.open(_filename, "w", true);

        if (!_f) {
            Serial.printf("Failed to open files\n");
            return false;
        }
        _state = LOGGING;

        return true;
    }

    void finalize() {
        // Update header with # of ticks

        // Append events file onto end of polled file

        // Move + rename file
    }

    /**
     * @brief Generates a vec of handles to passed data streams
     * @param streams A set of all datastreams
     * @return
     */
    static stream_handles_t create_stream_handles(streams_t streams, microseconds tick_interval, DLFStreamType type) {
        stream_handles_t h;

        for (size_t i = 0, e = streams.size(); i != e; ++i) {
            AbstractStream *ds = streams[i];

            if (ds->type() == type) {
                h.push_back(move(ds->handle(tick_interval, i)));
            }
        }

        return move(h);
    }
};
}  // namespace dlf