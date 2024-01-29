#pragma once

#include "AbstractStream.hpp"

namespace dlf::datastream {

class AbstractStreamHandle {
   protected:
    AbstractStream *stream;

    AbstractStreamHandle(AbstractStream *stream) : stream(stream) {}

   public:
    virtual bool available(dlf_tick_t tick) = 0;

    virtual void encode_into(std::vector<uint8_t> &buf, dlf_tick_t tick) = 0;

    virtual size_t encode_header_into(std::vector<uint8_t> &buf) = 0;
};

}  // namespace dlf::datastream

typedef std::unique_ptr<dlf::datastream::AbstractStreamHandle> stream_handle_t;
typedef std::vector<std::unique_ptr<dlf::datastream::AbstractStreamHandle>> stream_handles_t;