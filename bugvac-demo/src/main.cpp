#include <Arduino.h>
#include <FS.h>
#include <FastLED.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <dlf_logger.h>

#define PIN_VUSB_SENSE GPIO_NUM_27
#define RTC_VUSB_SENSE 17

CRGB leds[1];
TinyGPSPlus gps;

struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t centisecond;
} data_time;

struct {
    double lat;
    double lng;
    double alt;
    uint32_t satellites;
} pos;

CSCLogger logger(SD);

bool sleep_override = false;

void setup_logger() {
    auto time_log_interval = std::chrono::seconds(5);
    POLL(logger, data_time.year, time_log_interval);
    POLL(logger, data_time.month, time_log_interval);
    POLL(logger, data_time.day, time_log_interval);
    POLL(logger, data_time.hour, time_log_interval);
    POLL(logger, data_time.minute, time_log_interval);
    POLL(logger, data_time.second, time_log_interval);
    POLL(logger, data_time.centisecond, time_log_interval);

    auto sat_log_interval = std::chrono::seconds(5);
    POLL(logger, pos.satellites, sat_log_interval);

    auto pos_log_interval = std::chrono::seconds(1);
    POLL(logger, pos.lat, pos_log_interval);
    POLL(logger, pos.lng, pos_log_interval);
    POLL(logger, pos.alt, pos_log_interval);

    logger.begin();
}

void update_gps_vals() {
    if (gps.date.isUpdated() && gps.time.isUpdated()) {
        data_time.year = gps.date.year();
        data_time.month = gps.date.month();
        data_time.day = gps.date.day();
        data_time.hour = gps.time.hour();
        data_time.minute = gps.time.minute();
        data_time.second = gps.time.second();
        data_time.centisecond = gps.time.centisecond();
    }

    if (gps.satellites.isUpdated()) {
        pos.satellites = gps.satellites.value();
    }

    if (gps.location.isUpdated()) {
        pos.lat = gps.location.lat();
        pos.lng = gps.location.lng();
        pos.alt = gps.altitude.meters();
    }
}

void wait_sd() {
    SPI.begin(18, 19, 23);
    while (!SD.begin(5)) {
        FastLED.showColor(CRGB::Red);
        delay(500);
        FastLED.showColor(CRGB::Black);
        delay(500);
    }
    Serial.println("Card connected!");
}

void wait_valid_time() {
    while (!gps.time.isValid()) {
        FastLED.showColor(CRGB::Yellow);
        delay(500);
        FastLED.showColor(CRGB::Black);
        delay(500);
    }
    Serial.println("Valid time received!");
}

void task_gps(void* args) {
    Serial1.begin(9600, SERIAL_8N1, 22, 21);
    while (true) {
        // Read GPS
        while (Serial1.available())
            if (gps.encode(Serial1.read()))
                update_gps_vals();

        delay(100);
    }
}

bool has_usb_power() {
    return analogRead(PIN_VUSB_SENSE) >= 4095;
}

void task_shutdown_monitor(void* args) {
    vTaskDelay(1000);

    while (true) {
        bool io0_sleep = !digitalRead(0);
        bool usb_sleep = !sleep_override && !has_usb_power();

        if (io0_sleep || usb_sleep)
            break;

        Serial.println("TASK MONITOR");
        vTaskDelay(1000);
    }

    Serial.println("GOING TO SLEEP!");

    // Sleep GPS

    // Turn off LED
    FastLED.showColor(CRGB::Black);

    // Wake when USB is plugged in.
    esp_sleep_enable_ext0_wakeup(PIN_VUSB_SENSE, 1);

    // go to deep sleep.
    esp_deep_sleep_start();

    vTaskDelete(NULL);
    return;
}

run_handle_t r = 0;
void setup() {
    Serial.begin(115200);

    FastLED.addLeds<WS2812, 2, GRB>(leds, 1);
    FastLED.setBrightness(10);
    FastLED.showColor(CRGB::Red);

    // No USB power. Woke from manual reset, on battery power.
    // Prevent re-sleep.
    pinMode(PIN_VUSB_SENSE, INPUT_PULLDOWN);
    pinMode(0, INPUT_PULLUP);
    if (!has_usb_power()) {
        FastLED.showColor(CRGB::YellowGreen);  // overriden wake (battery + reset)
        sleep_override = true;
    } else {
        FastLED.showColor(CRGB::Blue);  // standard wake
    }

    xTaskCreate(task_shutdown_monitor, "gps", 4096, NULL, 5, NULL);
    xTaskCreate(task_gps, "gps", 4096, NULL, 5, NULL);

    // wait_sd();
    // wait_valid_time();
    // setup_logger();

    // Start a run
    // if (!sleep_override) {
    //     double m = 0.;
    //     r = logger.start_run(Encodable(m, "double"));
    // }

    // FastLED.showColor(CRGB::Green);
}

void loop()  // Writes GPS data to the Serial port with a baud rate of 115200
{
}
