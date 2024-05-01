// https://stackoverflow.com/questions/73340921/how-to-visualize-layout-of-c-struct-class
#include <Arduino.h>
#include <dlf_logger.h>
#include <SD_MMC.h>
#include <FastLED.h>

#define PIN_ARGB 26

using std::chrono::seconds;

bool led_state = 0;

CSCLogger logger(SD_MMC);

CRGB leds[3];


struct LMAO {
    uint8_t lel;
};

template <uint8_t DATA_PIN, EOrder RGB_ORDER = GRB>
class IN_PI42TAS : public ClocklessController<DATA_PIN, C_NS(300), C_NS(600), C_NS(200), RGB_ORDER, 4> {};

LMAO l;
long start = millis();
run_handle_t run;

struct a_struct {
    uint8_t a1;
    uint8_t a2;
} test;

void setup()
{
    FastLED.addLeds<IN_PI42TAS, PIN_ARGB, GRB>(leds, 3);

    Serial.begin(115200);
    Serial.println("Open SD");
    if (!SD_MMC.begin())
    {
        Serial.println("Failed to open SD");
    }

    Serial.printf("SD Initialized! Size: %lumb\n", SD_MMC.cardSize() / (1llu << 20));

    logger
        .poll(led_state, "led_poll", seconds(1))
        .watch(led_state, "led_event")
        .watch(start, "start_time")
        .watch(test, "Test struct");
    
    run = logger.start_run(0);
    start = millis();
}


CRGB blink_color = CRGB::Red;
void loop()
{
    leds[0] = blink_color;
    FastLED.show();
    led_state = true;
    delay(1000); // wait for a second

    leds[0] = CRGB::Black;
    FastLED.show();
    led_state = false;
    delay(1000); // wait for a second

    if(millis() - start > 5000 && run) {
        Serial.println("Stopping run!!!!!");
        blink_color = CRGB::Green; 
        logger.stop_run(run);
        run = 0;
    }
}