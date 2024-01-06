#include <vector>

#include "FS.h"
#include "dlf_cfg.h"
#include "dlf_datastream.h"
#include "dlf_types.h"
#include "uuid.h"

namespace dlf {
using namespace datastream;
using namespace std;

class LogFile {
   protected:
    dlf_streams_t _streams;

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
    template <typename meta_t, dlf_stream_type_e stream_type>
    LogFile(dlf_streams_t streams, meta_t meta, String &dir, FS &fs) : _streams(streams) {
        // Open run dir
        fs.mkdir(dir);
        File run_dir = fs.open(dir);
        if (!run_dir) return;

        // Create files for tuning and data.
        _uuid = StringUUIDGen();

        _base_f = run_dir.open(uuid + ".dlf", "w", true);
        _event_f = run_dir.open(uuid + ".events.tmp", "w", true);

        if (!_base_f || !_event_f) {
            Serial.printf("Failed to open files\n");
            return;
        }

        error = NONE;
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

    virtual bool begin() = 0;

    void write_header() {
        for (auto &stream : _streams) {
        }
    }

    bool begin() {
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

        while (self->is_logging && self->error == NONE) {
            // allow RX timeout to poll is_logging flag at a min time.
            BaseType_t res = xQueueReceive(self->q, buf, pdMS_TO_TICKS(100));
            if (res != pdTRUE) continue;

            // Write to SD
            if (!self->f) break;
            self->f.write(buf->data(), buf->size());

            // Dealloc vector
            delete buf;
            buf = NULL;
        }

        Serial.println("Writer task finished");
        vTaskDelete(NULL);
    }

    static void task_sampler(void *arg) {
        LogFile *self = static_cast<LogFile *>(arg);

        // Init sampling task and streams.
        self->_tick = 0;

        // Generate handles to data streams
        vector<unique_ptr<AbstractDataStreamHandle>> handles;
        for (size_t i = 0, e = self->_streams.size(); i != e; ++i) {
            AbstractDataStream *s = self->_streams[i];
            handles.push_back(move(s->handle(self->_tick_interval, i)));
        }

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

            // Fill data buffer
            while (buf->size() < DLF_MIN_SAMPLE_BUFFER) {
                for (auto &h : handles) {
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
};
}  // namespace dlf