#include <datastream/EventStreamHandle.hpp>

extern "C" {
// https://github.com/haipome/fnv/tree/master
#include <fnv.h>
}

using namespace dlf::datastream;

EventStreamHandle::EventStreamHandle(EventStream *stream, dlf_stream_idx_t idx) : AbstractStreamHandle(stream, idx) {
}

inline size_t EventStreamHandle::current_hash() {
    return fnv_32_buf(stream->data_source(), stream->data_size(), FNV1_32_INIT);
}

// This called every tick. current_hash uses FNV to try to be efficient, but
// if perf is an issue look for alternatives.
bool EventStreamHandle::available(dlf_tick_t tick) {
    bool a = _hash != current_hash();
#ifdef DEBUG
    DEBUG.printf(
        "\tCheck Event Data\n"
        "\t\tid: %s\n"
        "\t\tAvailable: %d\n",
        stream->id.c_str(), a);
#endif
    return a;
}

size_t EventStreamHandle::encode_header_into(StreamBufferHandle_t buf) {
#ifdef DEBUG
    DEBUG.printf(
        "\tEncode Event Header\n"
        "\t\tidx: %d\n"
        "\t\ttype_id: %s\n"
        "\t\tid: %s\n"
        "\t\tnotes: TODO\n",
        idx, stream->type_id.c_str(), stream->id.c_str());
#endif
    dlf_event_stream_header_t h;
    strlcpy(h.type_id, stream->type_id.c_str(), sizeof(h.type_id));
    strlcpy(h.id, stream->id.c_str(), sizeof(h.type_id));
    strlcpy(h.notes, "NOTES....", sizeof(h.type_id));

    return xStreamBufferSend(buf, &h, sizeof(h), portMAX_DELAY);
}

// FIXME: High possibility of overrunning streambuffer on initial tick (where all events are written)
// with lots of event data streams. Figure out how to mitigate!
size_t EventStreamHandle::encode_into(StreamBufferHandle_t buf, dlf_tick_t tick) {
#ifdef DEBUG
    DEBUG.printf(
        "\tEncode Event Data\n"
        "\t\tid: %s\n",
        stream->id.c_str());
#endif
    vTaskSuspendAll();  // Make sure hash and current written state stay in sync
    _hash = current_hash();

    // Write sample header
    dlf_event_stream_sample_t h;
    h.stream = idx;
    h.sample_tick = tick;
    xStreamBufferSend(buf, &h, sizeof(h), 0);

    // Write data sample
    size_t written = xStreamBufferSend(buf, stream->data_source(), stream->data_size(), 0);
    xTaskResumeAll();

    return written;
}