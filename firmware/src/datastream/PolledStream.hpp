#pragma once

#include <memory>

#include "AbstractStream.hpp"

namespace dlf::datastream {

/**
 * Concrete class representing metadata about a
 * stream of data that should be polled at some interval.
 */
class PolledStream : public AbstractStream {
   private:
    microseconds _sample_interval_us;
    microseconds _phase_us;

   public:
    template <typename T>
    PolledStream(T &dat, String id, microseconds sample_interval, microseconds phase = std::chrono::microseconds::zero())
        : AbstractStream(dat, id), _sample_interval_us(sample_interval), _phase_us(phase) {
    }

    stream_handle_t handle(microseconds tick_interval, dlf_stream_idx_t idx);

    dlf_stream_type_e type();
};
}  // namespace dlf::datastream