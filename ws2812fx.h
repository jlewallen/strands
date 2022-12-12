#pragma once

#include <FastLED.h>
#include <WS2812FX.h>

#include "secrets.h"

class Ws2812fxDriver {
private:
    WS2812FX ws2812fx_{ STRANDS_NUMBER_LEDS, STRANDS_DATA_PIN, NEO_RGB + NEO_KHZ800 };
    CRGB leds_[STRANDS_NUMBER_LEDS]; // Can we remove this eventually?
    CRGBPalette16 palette_{ CRGB::Black };
    CRGBPalette16 target_{ CRGB::Black };
    bool palettized_{ false };
    bool enabled_{ false };

public:
    void begin();

    void service(bool enabled);

private:
    void blank(uint32_t color);

public:
    bool isRunning();

    void start();

    void stop();

    uint8_t getBrightness();

    void setBrightness(uint8_t value);

    CRGBPalette16 getPalette();

    void setPalette(CRGBPalette16 palette);

    uint8_t getMode();

    void setMode(uint8_t value);

    uint16_t getSpeed();

    void setSpeed(uint16_t value);

    uint8_t getModeCount();

    const __FlashStringHelper *getModeName(uint8_t i);

    void setColor(uint32_t value);

public:
    uint16_t static_waves();
    uint16_t color_waves();
};