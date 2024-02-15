#pragma once

#include "AbstractStream.hpp"
#include <freertos/stream_buffer.h>

namespace dlf::datastream {

/**
 * @brief Provides access to the stream of data underlying an AbstractStream
 * 
 * Provides utilities for reading and encoding data at proper intervals into 
 * the formats defined by concrete classes.
 */
class AbstractStreamHandle {
   protected:
    AbstractStream *stream;
    dlf_stream_idx_t idx;

    AbstractStreamHandle(AbstractStream *stream, dlf_stream_idx_t idx) : stream(stream), idx(idx) {}

   public:
    virtual bool available(dlf_tick_t tick) = 0;

    virtual size_t encode_into(StreamBufferHandle_t buf, dlf_tick_t tick) = 0;

    virtual size_t encode_header_into(StreamBufferHandle_t buf) = 0;
};

}  // namespace dlf::datastream

typedef std::unique_ptr<dlf::datastream::AbstractStreamHandle> stream_handle_t;
typedef std::vector<std::unique_ptr<dlf::datastream::AbstractStreamHandle>> stream_handles_t;

