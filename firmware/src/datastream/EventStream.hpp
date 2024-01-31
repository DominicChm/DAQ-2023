#pragma once

#include <memory>

#include "AbstractStream.hpp"

using std::chrono::microseconds;

namespace dlf::datastream {

/**
 * Concrete class representing metadata about a
 * stream of data that should be polled at some interval.
 */
class EventStream : public AbstractStream {
   public:
    template <typename T>
    EventStream(T &dat, String id) : AbstractStream(dat, id) {
    }

    stream_handle_t handle(microseconds tick_interval, dlf_stream_idx_t idx);

    size_t size();

    dlf_stream_type_e type();
};
}  // namespace dlf::datastream
