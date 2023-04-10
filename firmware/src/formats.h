#include <Arduino.h>
#pragma once
struct fmt_BASE
{
    uint8_t id = 1;

    fmt_BASE(uint8_t id) : id(id) {}
};

struct fmt_accelerometer : public fmt_BASE
{
    uint8_t x;
    uint8_t y;
    uint8_t z;

    fmt_accelerometer() : fmt_BASE(1) {}
};
