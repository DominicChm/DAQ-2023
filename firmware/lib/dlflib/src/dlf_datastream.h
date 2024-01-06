#pragma once
#include <Arduino.h>
#include <string.h>

#include <chrono>
#include <vector>

#include "dlf_types.h"
#include "dlf_util.h"

using std::chrono::microseconds;

enum DLFStreamType {
    DLF_UNKNOWN,
    DLF_EVENT,
    DLF_POLLED,
};

namespace dlf::datastream {
/**
 * Abstract class representing a source of data as well as some information (name, typeID) about it.
 */
class AbstractDataStream {
   protected:
    const size_t _data_size;
    const void *_data_source;

    String *id;
    String *type_id;

    template <typename T>
    AbstractDataStream(T &dat, String id, String type_id = characteristic_type_name<T>()) : _data_size(sizeof(T)), _data_source(&dat) {
        dlf_assert(id.length() < member_sizeof(dlf_stream_header_t, id), "ID too long");
        dlf_assert(type_id.length() < member_sizeof(dlf_stream_header_t, type_id), "Type ID too long");
    }

   public:
    virtual std::unique_ptr<AbstractDataStreamHandle> handle(microseconds tick_interval, dlf_stream_idx_t idx) = 0;

    virtual size_t size() = 0;

    inline size_t data_size() {
        return _data_size;
    }

    inline const void *data_source() {
        return _data_source;
    }
};

class AbstractDataStreamHandle {
   protected:
    AbstractDataStream *stream;

    AbstractDataStreamHandle(AbstractDataStream *stream) : stream(stream) {}

   public:
    virtual bool available(dlf_tick_t tick) = 0;

    virtual void encode_into(std::vector<uint8_t> &buf, dlf_tick_t tick) = 0;

    virtual size_t encode_header_into(std::vector<uint8_t> &buf) = 0;
};

}  // namespace dlf::datastream

typedef std::vector<dlf::datastream::AbstractDataStream *> dlf_streams_t;
