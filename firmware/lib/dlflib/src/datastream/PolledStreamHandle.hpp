#pragma once

#include <memory>

#include "PolledStream.hpp"
#include "AbstractStreamHandle.hpp"

namespace dlf::datastream {

class PolledStreamHandle : public AbstractStreamHandle {
    dlf_tick_t _sample_interval_ticks;
    dlf_tick_t _sample_phase_ticks;

   public:
    PolledStreamHandle(PolledStream *stream, dlf_tick_t sample_interval_ticks, dlf_tick_t sample_phase);

    bool available(dlf_tick_t tick);

    size_t encode_header_into(std::vector<uint8_t> &buf);

    void encode_into(std::vector<uint8_t> &buf, dlf_tick_t tick);
};

}  // namespace dlf::datastream