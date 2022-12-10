#include "fastled.h"

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
        palette_number_ = addmod8(palette_number_, 1, gGradientPaletteCount);
        target_ = gGradientPalettes[palette_number_];
    }

    EVERY_N_MILLISECONDS(40) {
        nblendPaletteTowardPalette(current_, target_, 16);
    }

    EVERY_N_MILLISECONDS(20) {
        palette_color_waves(current_, leds_, STRANDS_NUMBER_LEDS);
    }

    EVERY_N_MILLISECONDS(10) {
        FastLED.show();
    }
}
