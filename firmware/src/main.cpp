#include <Arduino.h>
#include <RunManager.h>
#include <DataSource.h>
#include <communication/communication.h>
#include <type_name.h>

T_DATA daq_data;

typedef DataSource ds;
DataSource data_sources[] = {
    ds(500, daq_data.sus, "Suspension Pots"),
    ds(100, daq_data.electrons, "Electrons"),
};

RunManager rm(100, data_sources);

void setup()
{
    Serial.begin(115200);
    rm.init();
    rm.start_new_run();
}

void loop()
{
    // rm.loop();
}