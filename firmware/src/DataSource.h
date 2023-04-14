#include <Arduino.h>
#include <type_name.h>

#pragma once

class DataSource
{
    void *data_source;
    size_t data_size;

    const char *name;
    uint32_t type_id;
    uint32_t interval_ms;

    uint32_t cycle_interval;
    int32_t cycles;

public:
    /**
     * Constructs a new datasource. Note that the provided name's length is checked at compile time.
     * !! cycle_interval is in units of the base RunManager frequency. !!
     * For example, if the Run's base interval is 10ms and interval is 10,
     * this value will be recorded every 10 * 10 = 100ms
     */
    template <typename T, size_t NAME_LEN>
    DataSource(uint32_t interval_ms, T &dat, const char (&name)[NAME_LEN])
    {
        static_assert(NAME_LEN < 32, "Name is too long! Max length is 31 chars");

        this->data_source = &dat;
        this->data_size = sizeof(T);
        this->interval_ms = interval_ms;
        this->name = (const char *)&name;
        this->type_id = id<T>();
    }

    void reset_base_interval(uint32_t base_interval)
    {
        cycle_interval = (base_interval + interval_ms - 1) / base_interval;
        Serial.printf("Datasource %s will store every %d cycles, with type-id %X\n", name, cycle_interval, type_id);
        cycles = 0;
    }

    size_t cycle(uint8_t *buf)
    {
        if (--cycles > 0)
            return 0;
        cycles = cycle_interval;

        memcpy(buf, data_source, data_size);

        return data_size;
    }
};