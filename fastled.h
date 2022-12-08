#pragma once

// https://github.com/FastLED/FastLED/wiki/Interrupt-problems
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

#include <FastLED.h>

#include "palettes.h"
#include "secrets.h"

extern uint8_t gCurrentPaletteNumber;
extern CRGBPalette16 gCurrentPalette;
extern CRGBPalette16 gTargetPalette;

class FastLedDriver {
private:
    CRGB leds_[STRANDS_NUMBER_LEDS];

public:
    void begin();

    void service(bool enabled);

    void blank(uint32_t color);

private:
    void palettes();

    // This function draws color waves with an ever-changing, widely-varying set
    // of parameters, using a color palette.
    void color_waves(CRGB *ledarray, uint16_t num_leds, CRGBPalette16 &palette);

    // Alternate rendering function just scrolls the current palette across the
    // defined LED strip.
    void palette_test(CRGB *leds_array, uint16_t number_leds, CRGBPalette16 const &current_palette);
};