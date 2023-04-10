#include <Arduino.h>
#pragma once

struct FormatEntry
{
    uint8_t id;        // NOTE: Must be >0. ID 0 is reserved for NO TYPE. ID specifies datatype of struct.
    uint32_t interval; // ms between each record of this format
} __attribute__((packed));

struct Location
{
    float longitude;
    float latitude;
} __attribute__((packed));

struct RunHeader
{
    char name[256];            // Self-explanatory
    char description[512];     // Self-explanatory
    uint64_t start_time_epoch; // Start time of the run.
    FormatEntry formats[16];   // Formats contained in this file.
    Location location;         // Location where this run was done. Recorded at run start.
} __attribute__((packed));
