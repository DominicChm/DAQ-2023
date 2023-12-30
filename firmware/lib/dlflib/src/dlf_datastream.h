#pragma once
#include <Arduino.h>
#include <string.h>

#include <chrono>
#include <vector>

#include "dlf_types.h"
#include "dlf_util.h"

class DLFDataStream {
    const size_t _data_size;
    const void *_data_source;

   public:
    /**
     * Constructs a new datasource. Note that the provided name's length is checked at compile time.
     * !! cycle_interval is in units of the base RunManager frequency. !!
     * For example, if the Run's base interval is 10ms and interval is 10,
     * this value will be recorded every 10 * 10 = 100ms
     */
    template <typename T>
    DLFDataStream(T &dat, std::chrono::duration<dlf_tick_t, std::milli> sample_interval, const char *id, const char *type_id = characteristic_type_name<T>())
        : _data_size(sizeof(T)), _data_source(&dat) {
        dlf_assert(strlen(id) < sizeof(e.id), "ID too long");
        dlf_assert(strlen(type_id) < sizeof(e.type_id), "Name too long");
    }

    virtual size_t encode_into() = 0;
};