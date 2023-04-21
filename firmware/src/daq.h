#pragma once
#include <Arduino.h>
#include <DataSource.h>
#include <RunManager.h>
#include <communication/communication.h>

T_DATA daq_data;

uint8_t d0 = 0;
uint8_t d1 = 1;
struct sin_waves_t {
    float s5;
    float s10;
    float s1;
} sin_waves;
uint16_t d_random = 3;

typedef DataSource ds;
DataSource data_sources[] = {
    ds(1000, d0, "data0"),
    ds(3000, d1, "data1"),
    ds(0, sin_waves, "sin waves"),
    ds(0, d_random, "random data"),
};

RunManager rm(10, data_sources);
