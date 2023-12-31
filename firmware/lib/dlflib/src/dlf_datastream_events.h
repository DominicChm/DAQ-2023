#include <tr1/functional_hash.h>

#include <memory>

#include "dlf_datastream.h"

using std::chrono::microseconds;
using std::tr1::_Fnv_hash;

/**
 * Concrete class representing metadata about a
 * stream of data that should be polled at some interval.
 */
class DLFEventDataStream : DLFDataStream {
    friend class DLFEventDataStreamHandle;

    /**
     * Concrete class representing a handle to a
     * stream of data that should be polled at some interval.
     */
    class DLFEventDataStreamHandle : DLFDataStreamHandle {
        friend class DLFEventDataStream;
        size_t _hash;

        DLFEventDataStream &_stream;
        dlf_tick_t _sample_interval_ticks;
        dlf_tick_t _sample_phase_ticks;

        DLFEventDataStreamHandle(DLFEventDataStream &s) : _stream(s) {
        }

        size_t encode_header_into(char *buf) {
            return 0;
        }

        size_t encode_into(char *buf, int64_t tick) {
            size_t curr_hash = _Fnv_hash::hash(_stream._data_source, _stream._data_size);

            if (curr_hash == _hash) return 0;
            _hash = curr_hash;

            // encode data
            memcpy(buf, _stream._data_source, _stream._data_size);
        }
    };

   private:
    microseconds _sample_interval_us;
    microseconds _phase_us;

   public:
    template <typename T>
    DLFEventDataStream(T &dat, const char *id, microseconds sample_interval, microseconds phase = std::chrono::zero())
        : DLFDataStream(dat, id), _sample_interval_us(sample_interval), _phase_us(phase) {
    }

    std::unique_ptr<DLFDataStreamHandle> get_handle(microseconds tick_interval) {
        return std::unique_ptr<DLFDataStreamHandle>{new DLFEventDataStreamHandle(*this)};
    }
};