#include <ESPNtpClient.h> // gettimeofday, localtime

#include "web_server.h"

#include "ws2812fx.h"

#include "secrets.h"

LedDriver leds;
WebServer web_server;

#if defined(STRANDS_ENABLE_WIFI) && defined(STRANDS_ENABLE_SCHEDULE)
static bool enabled = false;
#else
static bool enabled = true;
#endif

void tests() {
#if defined(STRANDS_TEST_HALF) || defined(STRANDS_TEST_FULL)
    while (true) {
#if defined(STRANDS_TEST_FULL)
        FastLED.setBrightness(255);
#else
        FastLED.setBrightness(128);
#endif
        blank(CRGB(255, 255, 255));
        FastLED.delay(100);
    }
#endif
}

void setup() {
    Serial.begin(115200);
    pinMode(STRANDS_LED_PIN, OUTPUT);
    digitalWrite(STRANDS_LED_PIN, LOW);

    Serial.println("startup:begin!");

    tests();

    Serial.println("leds:begin");

    leds.begin();

#if defined(STRANDS_ENABLE_WIFI)

    Serial.println("web-server:begin");

    web_server.begin();
#endif

    Serial.println("startup:done");

    digitalWrite(STRANDS_LED_PIN, HIGH);
}

void loop() {
    leds.service(enabled);

#if defined(STRANDS_ENABLE_WIFI)
    web_server.service();
#endif

    EVERY_N_MILLISECONDS(5000) {
        Serial.print(millis());
        Serial.print(" | ");

#if defined(STRANDS_ENABLE_NTP)
        timeval moment;
        gettimeofday(&moment, NULL);
        tm *local_tm = localtime(&moment.tv_sec);

#if defined(STRANDS_ENABLE_SCHEDULE)
        if (web_server.have_time()) {
            // 4:30pm to 10pm
            if (((local_tm->tm_hour == 16 && local_tm->tm_min >= 30) || (local_tm->tm_hour >= 17)) &&
                (local_tm->tm_hour < 24)) {
                enabled = true;
            } else {
                enabled = false;
            }
        }
#endif

        Serial.print(local_tm->tm_hour);
        Serial.print(":");
        Serial.print(local_tm->tm_min);
        Serial.print(":");
        Serial.print(local_tm->tm_sec);
        Serial.print(".");
        Serial.print(moment.tv_usec);
        Serial.print(" ");

        if (!web_server.have_time()) {
            Serial.print("? ");
        }

        if (enabled) {
            Serial.print("ON");
        } else {
            Serial.print("OFF");
        }

        Serial.println();
#endif
    }
}
