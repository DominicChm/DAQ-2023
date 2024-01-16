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

struct task_stream_sampler_arg_t {
};

class LogFile {
   protected:
    streams_t _streams;
    FS *_fs;
    String _dir;

    File _event_f;
    File _base_f;

    QueueHandle_t _event_q = xQueueCreate(DLF_QUEUE_SIZE, sizeof(vector<uint8_t> *));
    QueueHandle_t _polled_q = xQueueCreate(DLF_QUEUE_SIZE, sizeof(vector<uint8_t> *));

    TaskHandle_t _sampler_task;  // Samples memory at correct interval
    TaskHandle_t _writer_task;   // Writes sensors to SD

    bool is_logging = true;
    dlf_file_err_e error = NONE;

    String _uuid;

    dlf_tick_t _tick;
    microseconds _tick_interval;

   public:
    LogFile(streams_t streams, String dir, FS *fs)
        : _streams(streams), _fs(fs), _dir(dir) {
    }

    size_t max_stream_size() {
        size_t ms = 0;
        for (auto s : _streams)
            ms = max(ms, s->size());

        return ms;
    }

    size_t max_block_size() {
        size_t bs = 0;
        for (auto const s : _streams)
            bs += s->size();

        return bs;
    }

    String uuid() {
        return _uuid;
    }

    explicit operator bool() const {
        return error == NONE;
    }

    void write_header() {
        for (auto &stream : _streams) {
        }
    }

    template <typename meta_t>
    bool begin(meta_t meta) {
        // Open run dir
        _fs->mkdir(_dir);

        // Create files for tuning and data.
        _uuid = StringUUIDGen();

        _base_f = _fs->open(_dir + "/" + _uuid + ".dlf", "w", true);
        _event_f = _fs->open(_dir + "/" + _uuid + ".events.tmp", "w", true);

        if (!_base_f || !_event_f) {
            Serial.printf("Failed to open files\n");
            return false;
        }

        error = NONE;

        is_logging = true;
        xTaskCreate(task_writer, "writer", 1024, this, 10, NULL);
        xTaskCreate(task_sampler, "sampler", 1024, this, 20, NULL);

        return true;
    }

    void assemble() {
    }

    void finalize() {
        // Update header with # of ticks

        // Append events file onto end of polled file

        // Move + rename file
    }

    /**
     * Samples enabled sources
     */
    static void task_writer(void *arg) {
        LogFile *self = static_cast<LogFile *>(arg);

        // while (self->is_logging && self->error == NONE) {
        //     // allow RX timeout to poll is_logging flag at a min time.
        //     BaseType_t res = xQueueReceive(self->q, buf, pdMS_TO_TICKS(100));
        //     if (res != pdTRUE) continue;

        //     // Write to SD
        //     if (!self->f) break;
        //     self->f.write(buf->data(), buf->size());

        //     // Dealloc vector
        //     delete buf;
        //     buf = NULL;
        // }

        Serial.println("Writer task finished");
        vTaskDelete(NULL);
    }

    /**
     * FreeRTOS task that samples data for this logfile.
     *
     * Data is sampled onto the heap, where pointers are fed into respective data-type queues.
     *
     * @param arg A LogFile pointer
     */
    static void task_sampler(void *arg) {
        LogFile *self = static_cast<LogFile *>(arg);

        // Init sampling task and streams.
        self->_tick = 0;

        // Generate handles to data streams based on their type
        // Streams are stored differently so handles must be
        // handled differently.
        stream_handles_t polled_handles = create_stream_handles(self->_streams, self->_tick_interval, DLF_POLLED);
        stream_handles_t event_handles = create_stream_handles(self->_streams, self->_tick_interval, DLF_POLLED);

        // Main logging loop
        while (self->is_logging && self->error == NONE) {
            // Allocate a new vec on the heap
            // to allow later passing through a queue
            auto buf = new vector<uint8_t>(DLF_MIN_SAMPLE_BUFFER * 2);
            if (buf == nullptr) {
                Serial.println("Out of heap mem :(");
                self->error = HEAP_FULL;
                break;
            }

            // Fill data buffers
            while (buf->size() < DLF_MIN_SAMPLE_BUFFER) {
                for (auto &h : polled_handles) {
                    if (h->available(self->_tick))
                        h->encode_into(*buf, self->_tick);
                }

                for (auto &h : event_handles) {
                    if (h->available(self->_tick))
                        h->encode_into(*buf, self->_tick);
                }
                self->_tick++;
            }

            // Transfer buffer to writer task through respective queues.
            BaseType_t res = xQueueSendToBack(self->_polled_q, buf, 0);
            if (res != pdTRUE) {
                // Data is read faster than the writer could go.
                Serial.println("No space in sampling queue!");
                self->error = QUEUE_FULL;
                break;
            }
        }

        // Clean up

        vTaskDelete(NULL);
    }

    /**
     * @brief Generates a vec of handles to passed data streams, filtered by the templated datastream type.
     * @tparam T The type of datastream to generate handles for
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