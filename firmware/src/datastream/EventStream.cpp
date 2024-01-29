#include "EventStream.hpp"

#include "EventStreamHandle.hpp"

using namespace dlf::datastream;

stream_handle_t EventStream::handle(microseconds tick_interval, dlf_stream_idx_t idx) {
    return std::unique_ptr<AbstractStreamHandle>(new EventStreamHandle(this));
}

size_t EventStream::size() {
    return data_size() + 5;
}

DLFStreamType EventStream::type() {
    return DLF_EVENT;
}
