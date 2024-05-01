#pragma once

#include <Arduino.h>
#include <string.h>

#include <chrono>
#include <memory>
#include <vector>
#include <dlf_type_strings.h>

#include "../dlf_cfg.h"
#include "../dlf_types.h"
#include "../dlf_util.h"

inline const char *stream_type_to_string(dlf_stream_type_e t) {
    switch (t) {
        case POLLED:
            return "polled";
        case EVENT:
            return "event";
        default:
            return "PROBLEM";
    }
}

namespace dlf::datastream {

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

    template <typename T>
    AbstractStream(T &dat, String id, const char *type_id = characteristic_type_name<T>())
        : _data_size(sizeof(T)), _data_source(reinterpret_cast<uint8_t *>(&dat)), id(id), type_id(type_id) {
        type_structure = "NO STRUCTURE FOUND";

        type_hash = hash_str(type_id);
        if (struct_strings.count(type_hash) > 0)
            type_structure = struct_strings.at(type_hash);
    }

   public:
    String id;
    String type_id;
    String type_structure;
    int type_hash;

    /**
     * @brief Creates a new, linked StreamHandle
     * @param tick_interval
     * @param idx
     * @return
     */
    virtual std::unique_ptr<AbstractStreamHandle> handle(microseconds tick_interval, dlf_stream_idx_t idx) = 0;

    virtual dlf_stream_type_e type() = 0;

    inline size_t data_size() {
        return _data_size;
    }

    inline const uint8_t *data_source() {
        return _data_source;
    }
};

typedef std::vector<AbstractStream *> streams_t;

}  // namespace dlf::datastream
