#pragma once

#include <Arduino.h>
#include <string.h>

#include <chrono>
#include <memory>
#include <vector>

#include "../dlf_cfg.h"
#include "../dlf_types.h"
#include "../dlf_util.h"

namespace dlf::datastream {
enum DLFStreamType : uint8_t {
    DLF_POLLED,
    DLF_EVENT,
};

const String &stream_type_to_string(DLFStreamType t) {
    switch (t) {
        case DLF_POLLED:
            return "polled";
        case DLF_EVENT:
            return "event";
        default:
            return "PROBLEM";
    }
}

using std::chrono::microseconds;

class AbstractStreamHandle;
typedef std::unique_ptr<AbstractStreamHandle> stream_handle_t;

/**
 * Abstract class representing a source of data as well as some information (name, typeID) about it.
 */
class AbstractStream {
   protected:
    const size_t _data_size;
    const uint8_t *_data_source;

    String *id;
    String *type_id;

    template <typename T>
    AbstractStream(T &dat, String id, String type_id = characteristic_type_name<T>())
        : _data_size(sizeof(T)), _data_source(reinterpret_cast<uint8_t *>(&dat)) {
        dlf_assert(id.length() < member_sizeof(dlf_stream_header_t, id), "ID too long");
        dlf_assert(type_id.length() < member_sizeof(dlf_stream_header_t, type_id), "Type ID too long");
    }

   public:
    virtual std::unique_ptr<AbstractStreamHandle> handle(microseconds tick_interval, dlf_stream_idx_t idx) = 0;

    virtual size_t size() = 0;

    virtual DLFStreamType type() = 0;

    inline size_t data_size() {
        return _data_size;
    }

    inline const uint8_t *data_source() {
        return _data_source;
    }
};

typedef std::vector<AbstractStream *> streams_t;

}  // namespace dlf::datastream