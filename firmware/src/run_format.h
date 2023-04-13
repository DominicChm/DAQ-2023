#include <Arduino.h>
#pragma once

struct run_data_source_t
{
    uint32_t cycle_interval; // ms between each record of this format
    uint8_t type_id;   // NOTE: Must be >0. ID 0 is reserved for NO TYPE. ID specifies datatype of struct.
    char name[32];

} __attribute__((packed));

struct run_location_t
{
    float longitude;
    float latitude;
} __attribute__((packed));

template <size_t NUM_SOURCES>
struct run_header_t
{
    char name[256];            // Self-explanatory
    char description[512];     // Self-explanatory
    uint64_t base_cycle_interval_ms;
    uint64_t start_time_epoch; // Start time of the run.
    run_location_t location;   // Location where this run was done. Recorded at run start.
    size_t num_sources;
    run_data_source_t entries[NUM_SOURCES]; // Formats contained in this file.
} __attribute__((packed));
