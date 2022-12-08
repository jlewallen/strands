#include "fastled.h"

uint8_t gCurrentPaletteNumber = 0;
CRGBPalette16 gCurrentPalette(CRGB::Black);
CRGBPalette16 gTargetPalette(gGradientPalettes[0]);

void FastLedDriver::begin() {
    FastLED.addLeds<STRANDS_FASTLED_TYPE, STRANDS_DATA_PIN, STRANDS_COLOR_ORDER>(leds_, STRANDS_NUMBER_LEDS)
        .setDither(STRANDS_BRIGHTNESS < 255);

    FastLED.setBrightness(STRANDS_BRIGHTNESS);
}

void FastLedDriver::service(bool enabled) {
    if (enabled) {
        palettes();
    } else {
        blank(CRGB(0, 0, 0));
    }
}

void FastLedDriver::blank(uint32_t color) {
    EVERY_N_MILLISECONDS(100) {
        for (auto i = 0u; i < STRANDS_NUMBER_LEDS; ++i) {
            leds_[i] = color;
        }

        FastLED.show();
    }
}

void FastLedDriver::palettes() {
    EVERY_N_SECONDS(STRANDS_SECONDS_PER_PALETTE) {
        gCurrentPaletteNumber = addmod8(gCurrentPaletteNumber, 1, gGradientPaletteCount);
        gTargetPalette = gGradientPalettes[gCurrentPaletteNumber];
    }

    EVERY_N_MILLISECONDS(40) {
        nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, 16);
    }

    EVERY_N_MILLISECONDS(20) {
        color_waves(leds_, STRANDS_NUMBER_LEDS, gCurrentPalette);
    }

    EVERY_N_MILLISECONDS(10) {
        FastLED.show();
    }
}

// This function draws color waves with an ever-changing, widely-varying set
// of parameters, using a color palette.
void FastLedDriver::color_waves(CRGB *ledarray, uint16_t num_leds, CRGBPalette16 &palette) {
    static uint16_t sPseudotime = 0;
    static uint16_t sLastMillis = 0;
    static uint16_t sHue16 = 0;

    uint8_t sat8 = beatsin88(87, 220, 250);
    uint8_t bright_depth = beatsin88(341, 96, 224);
    uint16_t brightness_thetainc16 = beatsin88(203, (25 * 256), (40 * 256));
    uint8_t ms_multiplier = beatsin88(147, 23, 60);

    uint16_t hue16 = sHue16; // gHue * 256;
    uint16_t hueinc16 = beatsin88(113, 300, 1500);

    uint16_t ms = millis();
    uint16_t delta_ms = ms - sLastMillis;
    sLastMillis = ms;
    sPseudotime += delta_ms * ms_multiplier;
    sHue16 += delta_ms * beatsin88(400, 5, 9);
    uint16_t brightness_theta16 = sPseudotime;

    for (uint16_t i = 0u; i < num_leds; i++) {
        hue16 += hueinc16;
        uint8_t hue8 = hue16 / 256;
        uint16_t h16_128 = hue16 >> 7;
        if (h16_128 & 0x100) {
            hue8 = 255 - (h16_128 >> 1);
        } else {
            hue8 = h16_128 >> 1;
        }

        brightness_theta16 += brightness_thetainc16;

        uint16_t b16 = sin16(brightness_theta16) + 32768;
        uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
        uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * bright_depth) / 65536;
        bri8 += (255 - bright_depth);

        uint8_t index = hue8;
        // index = triwave8(index);
        index = scale8(index, 240);

        CRGB new_color = ColorFromPalette(palette, index, bri8);

        uint16_t pixel_number = i;
        pixel_number = (num_leds - 1) - pixel_number;
        nblend(ledarray[pixel_number], new_color, 128);
    }
}

// Alternate rendering function just scrolls the current palette across the
// defined LED strip.
void FastLedDriver::palette_test(CRGB *leds_array, uint16_t number_leds, CRGBPalette16 const &current_palette) {
    static uint8_t start_index = 0;
    start_index--;

    fill_palette(leds_array, number_leds, start_index, (256 / STRANDS_NUMBER_LEDS) + 1, current_palette, 255,
                 LINEARBLEND);
}