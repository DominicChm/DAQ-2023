#pragma once
#include <Arduino.h>
#include <util.h>

#define HEADER_VERSION 0

struct run_data_source_t {
    uint32_t cycle_interval;  // ms between each record of this format
    char name[128];
    char type_name[128];

} __attribute__((packed));

struct run_location_t {
    float longitude;
    float latitude;
} __attribute__((packed));

template <size_t NUM_SOURCES>
struct run_header_t {
    uint32_t header_version;                 // Characteristic verion of header.
    uint32_t cycle_time_base_ms;             // Time base of the run.
    uint32_t start_time_epoch;               // Start time of the run.
    run_location_t location;                 // Location where this run was done. Recorded at run start.
    char name[1024];                         // Self-explanatory
    char description[4096];                  // Self-explanatory
    uint32_t checksum_intermediate;
    uint32_t num_entries;                    // Number of entries
    run_data_source_t entries[NUM_SOURCES];  // Formats contained in this file.
} __attribute__((packed));
