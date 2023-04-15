#include <Arduino.h>
#include <DataSource.h>
#include <RunManager.h>
#include <communication/communication.h>
#include <ui.h>
#include <util.h>

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

UI ui(Serial);
namespace commands {
void start_run(UI &ui) {
    rm.start_new_run();
}

void stop_run(UI &ui) {
    rm.finish_current_run();
}

void init() {
    ui.add_command("start", start_run);
    ui.add_command("stop", stop_run);
}
}  // namespace commands

void setup() {
    daq_data.electrons.voltage_sense36 = 0x01;
    daq_data.sus.bl_pot = 0x02;
    Serial.begin(115200);
    Serial.print("$ ");
    commands::init();
}

void loop() {
    ui.poll();
}