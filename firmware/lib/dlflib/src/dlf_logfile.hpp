#include <vector>

#include "FS.h"
#include "dlf_cfg.h"
#include "dlf_types.h"
#include "uuid.h"

class DLFLogFile {
   protected:
    dlf_streams_t _streams;

    File _event_f;
    File _polled_f;

    QueueHandle_t q = xQueueCreate(DLF_QUEUE_SIZE, sizeof(std::vector<uint8_t> *));

    TaskHandle_t _sampler_task;  // Samples memory at correct interval
    TaskHandle_t _writer_task;   // Writes sensors to SD

    bool is_logging = true;
    dlf_file_err_e error = NONE;

    String _uuid;
    String _polled_data_fname;
    String _event_data_fname;

   public:
    template <typename meta_t, dlf_stream_type_e stream_type>
    DLFLogFile(dlf_streams_t streams, meta_t meta, String &dir, FS &fs) : _streams(streams) {
        // Open run dir
        fs.mkdir(dir);
        File run_dir = fs.open(dir);
        if (!run_dir) return;

        // Create files for tuning and data.
        _uuid = StringUUIDGen();
        _polled_data_fname = uuid + ".polled.dlf";
        _event_data_fname = uuid + ".events.dlf";

        _polled_f = run_dir.open(_event_data_fname, "w", true);
        _event_f = run_dir.open(_polled_data_fname, "w", true);

        if (!_polled_f || !_event_f) {
            Serial.printf("Failed to open files\n");
            return;
        }

        write_event_header();
        write_polled_header();

        error = NONE;
    }

    String uuid() {
        return _uuid;
    }

    explicit operator bool() const {
        return error == NONE;
    }

    virtual bool begin() = 0;

    void write_event_header() {
        for (auto &stream : _streams) {
        }
    }

    void write_polled_header() {
        for (auto &stream : _streams) {
        }
    }

    bool begin() {
        is_logging = true;
        xTaskCreate(task_writer, "writer", 1024, this, 10, NULL);
        xTaskCreate(task_sampler, "sampler", 1024, this, 20, NULL);

        return true;
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
        DLFLogFile *self = static_cast<DLFLogFile *>(arg);
        std::vector<uint8_t> *buf;

        self->write_polled_header();

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
        DLFLogFile *self = static_cast<DLFLogFile *>(arg);

        while (self->is_logging && self->error == NONE) {
            std::vector<uint8_t> *buf = new std::vector<uint8_t>(DLF_MIN_BLOCK_SIZE);
            if (buf == NULL) {
                Serial.println("OUT OF HEAP MEMORY!");
                self->error = HEAP_FULL;
                break;
            }

            BaseType_t res = xQueueSend(self->q, buf, 0);
            if (res != pdTRUE) {
                // Data is read faster than the writer could go.
                Serial.println("No space in sampling queue!");
                self->error = QUEUE_FULL;
                break;
            }
        }

        vTaskDelete(NULL);
    }
};