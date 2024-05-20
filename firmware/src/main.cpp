// https://stackoverflow.com/questions/73340921/how-to-visualize-layout-of-c-struct-class
#include <Arduino.h>
#include <FastLED.h>
#include <SD_MMC.h>
#include <TinyGPSPlus.h>
#include <dlf_logger.h>

#define PIN_ARGB 26
#define PIN_LOGSW 32

#define PIN_GPS_TX_ESP_RX 19
#define PIN_GPS_RX_ESP_TX 21

using std::chrono::seconds;

TinyGPSPlus gps;

CSCLogger logger(SD_MMC);

struct GPSPos {
    double lat;
    double lon;
    double alt;
} pos;

run_handle_t run;

// LEDs
CRGB leds[3];
template <uint8_t DATA_PIN, EOrder RGB_ORDER = GRB>
class IN_PI42TAS : public ClocklessController<DATA_PIN, C_NS(300), C_NS(600), C_NS(200), RGB_ORDER, 4> {};

void setup() {
    pinMode(PIN_LOGSW, INPUT_PULLUP);

    FastLED.addLeds<IN_PI42TAS, PIN_ARGB, GRB>(leds, 3);
    FastLED.setBrightness(5);

    Serial.begin(115200);

    // setup GPS serial input
    Serial1.begin(4800, SERIAL_8N1, PIN_GPS_TX_ESP_RX, PIN_GPS_RX_ESP_TX, true);

    // Initialize SD card
    Serial.println("Open SD");
    if (!SD_MMC.begin()) {
        Serial.println("Failed to open SD");

        // Blink red for if SD mount failure
        while (true) {
            FastLED.showColor(CRGB::Red);
            delay(1000);
            FastLED.showColor(CRGB::Black);
            delay(1000);
        }
    }

    Serial.printf("SD Initialized! Size: %lumb\n", SD_MMC.cardSize() / (1llu << 20));

    // Setup logger to log GPS position struct
    logger.watch(pos, "GPSPos");

    // Setup synchronization
    logger
        .wifi("test_net", "12345678")
        .syncTo("812.us.to", 9235);

    logger.begin();

    // Wait for switch to be disabled
    while (!digitalRead(PIN_LOGSW))
        FastLED.showColor(CRGB::FairyLight);
        delay(100);

    FastLED.showColor(CRGB::Yellow);
}

void loop() {
    // Read GPS
    while (Serial1.available()) {
        gps.encode(Serial1.read());
    }

    // Update GPS data.
    if (gps.location.isUpdated()) {
        Serial.println("Got GPS data");
        pos.lat = gps.location.lat();
        pos.lon = gps.location.lng();
        pos.alt = gps.altitude.meters();
    }

    /** DAQ Below Here **/

    // Control log starting (when switch is on)
    if (!run && !digitalRead(PIN_LOGSW)) {
        Serial.println("Starting run!!!");

        // Write metadata and start run
        // sample every 100ms
        struct meta {
            uint32_t time = 0;
            uint64_t another = 5;
        } run_metadata;
        run = logger.start_run(run_metadata, std::chrono::milliseconds(100));

        // Show blue to indicate run start
        FastLED.showColor(CRGB::Blue);
        delay(100);

        // Control log stopping (when switch is off)
    } else if (run && digitalRead(PIN_LOGSW)) {
        Serial.println("Stopping run!!!");

        // Blocks until run is stopped
        logger.stop_run(run);
        run = 0;

        FastLED.showColor(CRGB::Yellow);
        delay(100);
    }

    delay(100);
}