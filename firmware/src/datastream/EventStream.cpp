#include <datastream/EventStream.hpp>
#include <datastream/EventStreamHandle.hpp>

using namespace dlf::datastream;

EventStream::EventStream(Encodable dat, String id) : AbstractStream(dat, id)
{
    Serial.printf("EVENT STREAM CONST ENC %s\n", dat.type_id);
}

stream_handle_t EventStream::handle(microseconds tick_interval, dlf_stream_idx_t idx)
{
    return std::unique_ptr<AbstractStreamHandle>(new EventStreamHandle(this, idx));
}

dlf_stream_type_e EventStream::type()
{
    return EVENT;
}
