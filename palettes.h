#pragma once

#include <FastLED.h>

extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;
extern const size_t gGradientPaletteBytes;

void palette_color_waves(CRGBPalette16 &palette, CRGB *leds, uint16_t number_leds);

void palette_test(CRGBPalette16 const &palette, CRGB *leds, uint16_t number_leds);