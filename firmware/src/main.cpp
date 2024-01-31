#include <Arduino.h>
#include <dlf_run.h>

#include <chrono>
// #include <dlf_types.h>
// #include <dlf_logger.h>
// #include <chrono>
#include <SD_MMC.h>

struct {
    uint8_t i1 = 0x01;
    uint8_t i2 = 0x02;
    uint8_t i3 = 0x03;
    uint8_t i4 = 0x04;
    uint8_t i5 = 0x05;
} polled;
int event_int = 0;

// DLFLogger logger(SD_MMC, "/logger1");

dlf::datastream::PolledStream ds1(polled, "test", std::chrono::seconds(1));
// dlf::datastream::EventStream ds2(event_int, "test2");

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

dlf::Run run(SD_MMC);

void setup() {
    Serial.begin(115200);
    Serial.println("Open SD");
    if (!SD_MMC.begin()) {
        Serial.println("Failed to open SD");
    }

    Serial.println("Init streams");
    dlf::datastream::streams_t streams;

    Serial.println("Push 1");
    streams.push_back(&ds1);

    // Serial.println("Push 2");
    // streams.push_back(&ds2);
    struct {
        int test = 1;
    } meta;

    Serial.println("Run begin");
    run.begin(std::move(streams), std::chrono::milliseconds(500), meta);
    delay(5000);
    run.close();
}

void loop() {
}