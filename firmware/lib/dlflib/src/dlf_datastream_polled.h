#include <memory>
#include "dlf_datastream.h"

using std::chrono::microseconds;

/**
 * Concrete class representing metadata about a
 * stream of data that should be polled at some interval.
 */
class DLFPolledDataStream : DLFDataStream {
    friend class DLFPolledDataStreamHandle;

    /**
     * Concrete class representing a handle to a
     * stream of data that should be polled at some interval.
     */
    class DLFPolledDataStreamHandle : DLFDataStreamHandle {
        friend class DLFPolledDataStream;

        DLFPolledDataStream &_stream;
        dlf_tick_t _sample_interval_ticks;
        dlf_tick_t _sample_phase_ticks;

        DLFPolledDataStreamHandle(DLFPolledDataStream &s, microseconds sample_interval_us, microseconds phase_us, microseconds tick_interval_us)
            : _stream(s) {
            // interval of 1 samples every tick which is max speed (anything % 1 == 0)
            // interval can't be 0 (causes div/0 err when using mod)
            _sample_interval_ticks = max(sample_interval_us.count() / tick_interval_us.count() + 1, 1ll);
            _sample_phase_ticks = phase_us.count() / tick_interval_us.count();
        }

        size_t encode_header_into(char *buf) {
            return 0;
        }

        size_t encode_into(char *buf, int64_t tick) {
            if ((tick + _sample_phase_ticks) % _sample_interval_ticks != 0) return 0;

            // Sample data
            memcpy(buf, _stream._data_source, _stream._data_size);
        }
    };

   private:
    microseconds _sample_interval_us;
    microseconds _phase_us;

   public:
    template <typename T>
    DLFPolledDataStream(T &dat, const char *id, microseconds sample_interval, microseconds phase = std::chrono::zero())
        : DLFDataStream(dat, id), _sample_interval_us(sample_interval), _phase_us(phase) {
    }

    std::unique_ptr<DLFDataStreamHandle> get_handle(microseconds tick_interval) {
        return std::unique_ptr<DLFDataStreamHandle>{new DLFPolledDataStreamHandle(
            *this,
            _sample_interval_us,
            _phase_us,
            tick_interval)};
    }
};