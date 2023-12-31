#pragma once
#include <Arduino.h>
#include <string.h>

#include <chrono>
#include <vector>

#include "dlf_types.h"
#include "dlf_util.h"

using std::chrono::microseconds;

/**
 * Abstract class representing a source of data as well as some information (name, typeID) about it.
 */
class DLFDataStream {
   protected:
    const size_t _data_size;
    const void *_data_source;

    const char *id;
    const char *type_id;

    /**
     * Provides a unified interface for reading from DLFDataStreams
     */
    class DLFDataStreamHandle {
       public:
        virtual size_t encode_into(char *buf, int64_t tick) = 0;
    };
    friend class DLFDataStreamHandle;

   public:
    template <typename T>
    DLFDataStream(T &dat, String id, String type_id = characteristic_type_name<T>()) : _data_size(sizeof(T)), _data_source(&dat) {
        dlf_assert(strlen(id) < sizeof(e.id), "ID too long");
        dlf_assert(strlen(type_id) < sizeof(e.type_id), "Name too long");
    }

    virtual size_t encode_header_into(char *buf) = 0;

    virtual std::unique_ptr<DLFDataStreamHandle> get_handle(microseconds tick_interval);
};

