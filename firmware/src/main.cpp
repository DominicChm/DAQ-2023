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

uint8_t event_uint = 0x01;
uint8_t event_uint2 = 0x10;

dlf::datastream::PolledStream ds1(polled, "Polled 1", std::chrono::seconds(1));
dlf::datastream::EventStream ds2(event_uint, "Event 2");
dlf::datastream::EventStream ds3(event_uint2, "Event 3");

dlf::Run run(SD_MMC);


void setup() {
    Serial.begin(115200);
    Serial.println("Open SD");
    if (!SD_MMC.begin()) {
        Serial.println("Failed to open SD");
    }

    Serial.println("Init streams");
    dlf::datastream::streams_t streams;

    streams.push_back(&ds1);
    streams.push_back(&ds2);
    streams.push_back(&ds3);

    struct {
        int test = 1;
    } meta;

    Serial.printf("Run begin! Heap: %u\n", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
    run.begin(std::move(streams), std::chrono::milliseconds(500), meta);

    delay(2500);
    Serial.println("Modifying event stream 1!");
    event_uint = 0x02;

    delay(2500);
    Serial.println("Modifying event stream 2!");
    event_uint2 = 0x20;

    delay(5000);
    run.close();

    Serial.printf("Run done! Heap: %u\n", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
}

void loop() {
}