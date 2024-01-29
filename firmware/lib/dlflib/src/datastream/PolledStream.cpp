#include "PolledStream.hpp"

#include "PolledStreamHandle.hpp"

using namespace dlf::datastream;



stream_handle_t PolledStream::handle(microseconds tick_interval, dlf_stream_idx_t idx) {
    dlf_tick_t sample_interval_ticks = max(_sample_interval_us.count() / tick_interval.count() + 1, 1ll);
    dlf_tick_t sample_phase_ticks = _phase_us.count() / tick_interval.count();

    return std::unique_ptr<AbstractStreamHandle>(new PolledStreamHandle(
        this,
        sample_interval_ticks,
        sample_phase_ticks));
}

size_t PolledStream::size() {
    return data_size();
}

DLFStreamType PolledStream::type() {
    return DLF_EVENT;
}