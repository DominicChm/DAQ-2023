#include <Arduino.h>
#include <util.h>
#pragma once

struct run_data_source_t
{
    uint32_t cycle_interval; // ms between each record of this format
    char *name;
    char *type_name;

} __attribute__((packed));

struct run_location_t
{
    float longitude;
    float latitude;
} __attribute__((packed));

template <size_t NUM_SOURCES>
struct run_header_t
{
    uint32_t header_size;                   // Self-explanatory
    uint64_t cycle_time_base_ms;            // Time base of the run.
    uint64_t start_time_epoch;              // Start time of the run.
    run_location_t location;                // Location where this run was done. Recorded at run start.
    char *name;                             // Self-explanatory
    char *description;                      // Self-explanatory
    run_data_source_t entries[NUM_SOURCES]; // Formats contained in this file.

    run_header_t()
    {
    }
    
    run_header_t(const char *name, const char *description)
    {
        str2heap(&(this->name), name);
        str2heap(&(this->description), description);
    }

    ~run_header_t()
    {
        free(name);
        free(description);
    }

    void set_name(const char *name)
    {
        str2heap(&(this->name), name);
    }

    void set_description(const char *description)
    {
        str2heap(&(this->description), description);
    }

} __attribute__((packed));
