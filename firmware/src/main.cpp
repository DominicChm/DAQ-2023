#include <Arduino.h>
#include <daq.h>
#include <ui.h>
#include <util.h>
#include <web.h>

UI ui(Serial);
namespace commands {
void start_run(UI &ui) {
    rm.start_new_run();
}

void stop_run(UI &ui) {
    rm.finish_current_run();
}

void set_timebase(UI &ui) {
    int tbase = ui.positional<int>();

    if (ui.parse_error()) return;

    ui.printfn("T base %d", tbase);
}

void debug(UI &ui) {
    // ui.printfn("MIN_BLOCK_SIZE: %d", rm.min_block_size());
    // ui.printfn("BLOCK_SIZE: %d", rm.block_size());
}

void init() {
    ui.add_command("start", start_run);
    ui.add_command("stop", stop_run);
    ui.add_command("set_tbase", set_timebase);
    ui.add_command("debug", debug);
}
}  // namespace commands

// void write_dev() {
//     // yes, i'm manually writing json. deal with it.
//     File f = SD_MMC.open("/__DEV", FILE_WRITE);
//     f.println("{");
//     f.printf("\"header_size\": %u,", rm.header_size());
//     f.println("}");
// }

void setup() {
    daq_data.electrons.voltage_sense36 = 0x01;
    daq_data.sus.bl_pot = 0x02;
    Serial.begin(115200);
    Serial.print("$ ");
    commands::init();
    web::init();
}

void loop() {
    ui.poll();
}