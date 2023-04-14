#include <Arduino.h>
#include <util.h>

#pragma once

class DataSource
{
    void *_data_source;
    size_t _data_size;

    const char *_name;
    const char *_type_name;
    uint32_t _interval_ms;

    uint32_t _cycle_time_base_ms;
    int32_t _cycles;

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

        this->_data_source = &dat;
        this->_data_size = sizeof(T);
        this->_interval_ms = interval_ms;
        this->_name = (const char *)&name;
        this->_type_name = characteristic_type_name<T>();
    }

    void reset_base_interval(uint32_t base_interval)
    {
        _cycle_time_base_ms = (base_interval + _interval_ms - 1) / base_interval;
        _cycles = 0;
        Serial.printf("Datasource %s will store every %d cycles, with type-id %s\n", _name, _cycle_time_base_ms, _type_name);
    }

    size_t cycle(uint8_t *buf)
    {
        if (--_cycles > 0)
            return 0;
        _cycles = _cycle_time_base_ms;

        memcpy(buf, _data_source, _data_size);

        return _data_size;
    }

    constexpr const char *get_type_name()
    {
        return _type_name;
    }

    run_data_source_t header_entry()
    {
        run_data_source_t e = {0};
        e.cycle_interval = _cycle_time_base_ms;
        strlcpy(e.name, _name, sizeof(e.name));
        strlcpy(e.type_name, _type_name, sizeof(e.type_name));

        return e;
    }
};