#pragma once
#include <Arduino.h>
#include <DataSource.h>
#include <RunManager.h>
#include <communication/communication.h>

T_DATA daq_data;

uint8_t d0 = 0;
uint8_t d1 = 1;
uint8_t d15 = 15;
uint8_t d2 = 2;
uint8_t d3 = 3;

typedef DataSource ds;
DataSource data_sources[] = {
    ds(1000, d0, "data0"),
    ds(1000, d1, "data1"),
    ds(1500, d15, "data1.5"),
    ds(2000, d2, "data2"),
    ds(3000, d3, "data3"),
};

RunManager rm(1000, data_sources);
