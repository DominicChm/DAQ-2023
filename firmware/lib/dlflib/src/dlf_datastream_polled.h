#include <memory>

#include "dlf_datastream.h"

using std::chrono::microseconds;
namespace dlf::datastream {

/**
 * Concrete class representing metadata about a
 * stream of data that should be polled at some interval.
 */
class PolledDataStream : public AbstractDataStream {
   private:
    microseconds _sample_interval_us;
    microseconds _phase_us;

    template <typename T>
    PolledDataStream(T &dat, const char *id, microseconds sample_interval, microseconds phase = std::chrono::microseconds::zero())
        : DLFDataStream(dat, id), _sample_interval_us(sample_interval), _phase_us(phase) {
    }

    std::unique_ptr<AbstractDataStreamHandle> init(microseconds tick_interval, dlf_stream_idx_t idx) {
        dlf_tick_t sample_interval_ticks = max(_sample_interval_us.count() / tick_interval.count() + 1, 1ll);
        dlf_tick_t sample_phase_ticks = _phase_us.count() / tick_interval.count();

        return std::unique_ptr<AbstractDataStreamHandle>(new PolledDataStreamHandle(
            this,
            sample_interval_ticks,
            sample_phase_ticks));
    }

    size_t size() {
        return stream->data_size();
    }
};

class PolledDataStreamHandle : public AbstractDataStreamHandle {
    dlf_tick_t _sample_interval_ticks;
    dlf_tick_t _sample_phase_ticks;

   public:
    PolledDataStreamHandle(PolledDataStream *stream, dlf_tick_t sample_interval_ticks, dlf_tick_t sample_phase)
        : AbstractDataStreamHandle(stream), _sample_interval_ticks(sample_interval_ticks), _sample_phase_ticks(sample_phase) {
    }

    bool available(dlf_tick_t tick) {
        return ((tick + _sample_phase_ticks) % _sample_interval_ticks) != 0;
    }

    size_t encode_header_into(std::vector<uint8_t> &buf) {
        return 0;
    }

    void encode_into(std::vector<uint8_t> &buf, dlf_tick_t tick) {
        // Sample data
        buf.insert(buf.end(), stream->data_source(), stream->data_source() + stream->data_size());
    }
};

}  // namespace dlf::datastream