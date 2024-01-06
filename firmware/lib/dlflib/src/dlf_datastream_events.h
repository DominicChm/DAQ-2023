#include <tr1/functional_hash.h>

#include <memory>

#include "dlf_datastream.h"

using std::chrono::microseconds;
using std::tr1::_Fnv_hash;


class DLFEventDataStream : AbstractDataStream {
   private:
    size_t _hash;
    dlf_stream_idx_t _idx;

   public:
    template <typename T>
    DLFEventDataStream(T &dat, const char *id)
        : DLFDataStream(dat, id), _sample_interval_us(sample_interval), _phase_us(phase) {
    }

    void init(microseconds tick_interval, dlf_stream_idx_t idx) {
        _idx = idx;
    }

    bool available(dlf_tick_t tick) {
        return _hash != _Fnv_hash::hash(_data_source, _data_size);
    }

    size_t encode_header_into(char *buf) {
        return 0;
    }

    size_t size() {
        return _data_size + 5;
    }

    void encode_into(char *buf, dlf_tick_t tick) {
        // Sample data
        memcpy(buf, _data_source, _data_size);
        _hash = _Fnv_hash::hash(_data_source, _data_size);
    }
};