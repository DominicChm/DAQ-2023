#include <datastream/PolledStream.hpp>
#include <datastream/PolledStreamHandle.hpp>

using namespace dlf::datastream;

stream_handle_t PolledStream::handle(microseconds tick_interval, dlf_stream_idx_t idx) {
    Serial.printf("%d\n", tick_interval.count());
    dlf_tick_t sample_interval_ticks = max(_sample_interval_us / tick_interval, 1ll);
    dlf_tick_t sample_phase_ticks = _phase_us / tick_interval;

    return std::unique_ptr<AbstractStreamHandle>(new PolledStreamHandle(
        this,
        sample_interval_ticks,
        sample_phase_ticks));
}

size_t PolledStream::size() {
    return data_size();
}

dlf_stream_type_e PolledStream::type() {
    return POLLED;
}
