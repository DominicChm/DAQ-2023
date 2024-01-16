#pragma once

#include "AbstractStreamHandle.hpp"
#include "EventStream.hpp"

namespace dlf::datastream {
class EventStreamHandle : public AbstractStreamHandle {
    size_t _hash = 0;

   public:
    EventStreamHandle(EventStream *stream);

    size_t encode_header_into(std::vector<uint8_t> &buf);

    inline size_t current_hash();

    bool available(dlf_tick_t tick);

    void encode_into(std::vector<uint8_t> &buf, dlf_tick_t tick);
};
}  // namespace dlf::datastream
