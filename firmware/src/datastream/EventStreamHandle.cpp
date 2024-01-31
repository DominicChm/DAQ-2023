#include <datastream/EventStreamHandle.hpp>

extern "C" {
// https://github.com/haipome/fnv/tree/master
#include <fnv.h>
}

using namespace dlf::datastream;

EventStreamHandle::EventStreamHandle(EventStream *stream) : AbstractStreamHandle(stream) {
}

inline size_t EventStreamHandle::current_hash() {
    return fnv_32_buf(stream->data_source(), stream->data_size(), FNV1_32_INIT);
}

bool EventStreamHandle::available(dlf_tick_t tick) {
    return _hash != current_hash();
}

size_t EventStreamHandle::encode_header_into(StreamBufferHandle_t buf) {
    dlf_stream_header_t h;
    strncpy(h.id, "ID", sizeof(h.id));
    strncpy(h.type_id, "TYPE ID", sizeof(h.id));
    strncpy(h.notes, "NOTES", sizeof(h.id));

    return xStreamBufferSend(buf, &h, sizeof(h), portMAX_DELAY);
}

size_t EventStreamHandle::encode_into(StreamBufferHandle_t buf, dlf_tick_t tick) {
    vTaskSuspendAll();  // Make sure hash and current written state stay in sync
    _hash = current_hash();

    // Sample data
    size_t written = xStreamBufferSend(buf, stream->data_source(), stream->data_size(), 0);
    xTaskResumeAll();

    return written;
}