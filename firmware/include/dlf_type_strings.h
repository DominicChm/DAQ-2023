#ifndef _TYPE_STRINGS_H
#define _TYPE_STRINGS_H
#include <dlf_util.h>

#include <map>

// TODO: AUTOGENERATE THIS FILE
// format: "type_name;field_name:field_type:field_offset;field_name.subfield_name:field_type:field_offset"
const std::map<int, const char*> struct_strings = {
    {0x34ea9d1e, "bool"},
    {0x709634c9, "uint32_t"},
    {0x7f4b9d19, "a_struct;a1:uint8_t:0;a2:uint8_t:1"},
    {0x7022f86d, "meta_struct;time:uint32_t:0;another:uint64_t:8"},
    {0x9d7b8218, "gps_pos;lat:double:0;lon:double:8;alt:double:16"},
};

inline const char* get_structure_from_type_id(const char* type_name) {
    int type_name_hash = hash_str(type_name);

    if (struct_strings.count(type_name_hash) <= 0)
        return "!NO_STRUCTURE";

    return struct_strings.at(type_name_hash);
}

template <typename T>
inline const char* get_structure() {
    const char* type_id = t<T>();
    return get_structure_from_type_id(type_id);
}

#endif