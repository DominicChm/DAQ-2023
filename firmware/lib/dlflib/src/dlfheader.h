#pragma once
#include <Arduino.h>
#include <util.h>

#define HEADER_VERSION 0

struct dlf_stream_header_t {
    char type_id[32];        // Data type identifier. Identifies type of contained data.
    char id[32];             // Unique identifier for this specific stream
    uint32_t tick_interval;  // Interval on which this stream is collected.
    uint32_t tick_phase;     // Tick offset defining when this stream starts
    char notes[128];         // Anything that needs to be communicated about this data stream.
};
template <typename META_T, size_t NUM_SOURCE>
struct dlf_header_t {
    uint32_t magic = 0x8414;  // IDs DLF files. Also allows auto-detection of LSB/MSB encoding.
    uint32_t tick_base_us;    // Base time interval, in us. Limits how fast samples will be stored.

    char application[32];                 // An arbitrary application-specific identifier. Used to select a metadata parser.
    uint32_t meta_size = sizeof(meta_t);  // Metadata size stored in case there is no metadata parser available
    meta_t meta;                          // Metadata. Can be application-specific

    uint32_t num_streams;
    dlf_stream_header_t streams[num_streams];
    uint16_t meta_checksum;  // Checksum over metadata.
}