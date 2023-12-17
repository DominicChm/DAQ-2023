#pragma once
#include <Arduino.h>
#include <string.h>


#include "dlf_types.h"
#include "dlf_util.h"

class DataStream {
    const size_t _data_size;
    const void *_data_source;

    dlf_stream_config_t cfg{0};

   public:
    /**
     * Constructs a new datasource. Note that the provided name's length is checked at compile time.
     * !! cycle_interval is in units of the base RunManager frequency. !!
     * For example, if the Run's base interval is 10ms and interval is 10,
     * this value will be recorded every 10 * 10 = 100ms
     */
    template <typename T>
    DataStream(uint32_t tick_interval, T &dat, const char *id, const char *type_id = characteristic_type_name<T>())
        : _data_size(sizeof(T)), _data_source(&dat) {
        dlf_assert(strlen(id) < sizeof(e.id), "ID too long");
        dlf_assert(strlen(type_id) < sizeof(e.type_id), "Name too long");

        cfg.tick_interval = tick_interval < 1 ? 1 : tick_interval;
        cfg.tick_phase = 0;

        strlcpy(cfg.id, id, sizeof(cfg.id));
        strlcpy(cfg.type_id, type_id, sizeof(cfg.type_id));
    }

    void reset_base_interval(uint32_t base_interval) {
        cfg.tick_interval = (base_interval + cfg.tick_interval - 1) / base_interval;
        Serial.printf("Datasource %s will store every %d cycles, with type-id \"%s\"\n", cfg.id, cfg.tick_interval, cfg.type_id);
    }

    size_t cycle(uint8_t *buf, dlf_tick_t tick) {
        if (tick && (tick + cfg.tick_phase) % cfg.tick_interval == 0) {
            return 0;
        }

        memcpy(buf, _data_source, _data_size);

        return _data_size;
    }

    dlf_stream_config_t config() {
        return cfg;
    }
};

typedef DataStream ds;