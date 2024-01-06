#include <Arduino.h>
#include <dlf_datastream_events.h>
#include <dlf_datastream_polled.h>

// #include <dlf_types.h>
// #include <dlf_logger.h>
// #include <chrono>
#include <SD_MMC.h>

int polled_int = 0;
int event_int = 0;

// DLFLogger logger(SD_MMC, "/logger1");

DLFPolledDataStream ds1(polled_int, "test", std::chrono::milliseconds(10));
DLFEventDataStream ds2(event_int, "test2");

// DLFDataStream st_test(d, std::chrono::milliseconds(10), "test");

// // #include <dlf.h>

// typedef DLFDataStream ds;
// DLFDataStream data_sources[] = {
//     ds(1000, d0, "data0"),
//     ds(3000, d1, "data1"),
//     ds(0, sin_waves, "sin waves"),
//     ds(0, d_random, "random data"),
// };

// struct meta_t {
//     char name[128];
//     char desc[1024];
// }

// DLFLogger rm(10, data_sources);

// struct Data1 {
//     uint32_t u;
//     double d;
// };

// uint32_t u;

// void setup() {
//     Serial.begin(115200);
// }

// void loop() {
//     ui.poll();

//     // Generate some fun random data.
//     sin_waves = {
//         (float)sin(2. * M_PI * millis() / 5000) * 50 - 10,
//         (float)sin(2. * M_PI * millis() / 10000) * 10,
//         (float)sin(2. * M_PI * millis() / 1000) * 120 + 10};
//     d_random = random(1337);

//     delay(10);
// }

void setup() {
}

void loop() {
}