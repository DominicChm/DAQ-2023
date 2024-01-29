#include "EventStreamHandle.hpp"

extern "C" {
// https://github.com/haipome/fnv/tree/master
#include "fnv/fnv.h"
}

using namespace dlf::datastream;

EventStreamHandle::EventStreamHandle(EventStream *stream) : AbstractStreamHandle(stream) {
}

size_t EventStreamHandle::encode_header_into(std::vector<uint8_t> &buf) {
    return 0;
}

inline size_t EventStreamHandle::current_hash() {
    return fnv_32_buf(stream->data_source(), stream->data_size(), FNV1_32_INIT);
}

bool EventStreamHandle::available(dlf_tick_t tick) {
    return _hash != current_hash();
}

void EventStreamHandle::encode_into(std::vector<uint8_t> &buf, dlf_tick_t tick) {
    // Sample data
    buf.insert(buf.end(), stream->data_source(), stream->data_source() + stream->data_size());

    _hash = current_hash();
}