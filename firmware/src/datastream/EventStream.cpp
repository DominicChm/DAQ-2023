#include <datastream/EventStream.hpp>
#include <datastream/EventStreamHandle.hpp>

using namespace dlf::datastream;

stream_handle_t EventStream::handle(microseconds tick_interval, dlf_stream_idx_t idx) {
    return std::unique_ptr<AbstractStreamHandle>(new EventStreamHandle(this, idx));
}

size_t EventStream::size() {
    return data_size() + 5;
}

dlf_stream_type_e EventStream::type() {
    return EVENT;
}
