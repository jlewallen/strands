#pragma once

// https://github.com/FastLED/FastLED/wiki/Interrupt-problems
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

#include <FastLED.h>

#include "palettes.h"
#include "secrets.h"

class FastLedDriver {
private:
    uint8_t palette_number_{ 0 };
    CRGBPalette16 current_{ CRGB::Black };
    CRGBPalette16 target_{ gGradientPalettes[0].p };
    CRGB leds_[STRANDS_NUMBER_LEDS];

public:
    void begin();

    void service(bool enabled);

    void blank(uint32_t color);

private:
    void palettes();
};