#pragma once

#include <WS2812FX.h>

#include "secrets.h"

class Ws2812fxDriver {
private:
    WS2812FX ws2812fx_{ STRANDS_NUMBER_LEDS, STRANDS_DATA_PIN, NEO_RGB + NEO_KHZ800 };

public:
    void begin();

    void service(bool enabled);

    void blank(uint32_t color);

public:
    uint8_t getBrightness();

    void setBrightness(uint8_t value);

    uint8_t getMode();

    void setMode(uint8_t value);

    uint16_t getSpeed();

    void setSpeed(uint16_t value);

    uint8_t getModeCount();

    const __FlashStringHelper *getModeName(uint8_t i);

    void setColor(uint32_t value);
};