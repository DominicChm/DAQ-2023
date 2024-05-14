#ifndef DLF_ENCODABLE_H
#define DLF_ENCODABLE_H

#include <Arduino.h>
#include "dlf_util.h"
#include <dlf_type_strings.h>

class Encodable
{

public:
    const char *type_id;
    const char *type_structure;
    const size_t type_hash;
    const uint8_t *data;
    const size_t data_size;

    template <typename T>
    Encodable(T &v) : type_id(characteristic_type_name<T>()),
                      type_structure(get_structure_from_type_id(type_id)),
                      type_hash(hash_str(type_id)),
                      data(reinterpret_cast<uint8_t *>(&v)),
                      data_size(sizeof(T))
    {
    }

};

#endif