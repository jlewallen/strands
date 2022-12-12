#include "ws2812fx.h"
#include "palettes.h"

static Ws2812fxDriver *driver_ = nullptr;

uint16_t fl_color_waves(void) {
    return driver_->color_waves();
}

uint16_t fl_static_waves(void) {
    return driver_->static_waves();
}

void Ws2812fxDriver::begin() {
    driver_ = this;

    ws2812fx_.init();
    ws2812fx_.setCustomMode((uint8_t)0, FSH("FL: Color Waves"), fl_color_waves);
    ws2812fx_.setCustomMode((uint8_t)1, FSH("FL: Static Waves"), fl_static_waves);
    ws2812fx_.setSpeed(1000);
    blank(0);
    ws2812fx_.start();
}

void Ws2812fxDriver::service(bool enabled) {
    if (enabled_ != enabled) {
        enabled_ = enabled;

        Serial.println("ws2812fx: enabling");

        if (enabled_) {
            setMode(FX_MODE_STATIC);
#if defined(STRANDS_WS2812FX_DEFAULT_MODE)
            ws2812fx_.setMode(STRANDS_WS2812FX_DEFAULT_MODE);
#endif
#if defined(STRANDS_WS2812FX_DEFAULT_COLOR)
            setColor(STRANDS_WS2812FX_DEFAULT_COLOR);
#endif
            setBrightness(STRANDS_BRIGHTNESS);
        } else {
            setBrightness(0);
            blank(0);
        }

        Serial.println("ws2812fx: enabled");
    }

    if (palettized_) {
        EVERY_N_MILLISECONDS(40) {
            nblendPaletteTowardPalette(palette_, target_, 16);
        }

        EVERY_N_MILLISECONDS(20) {
            palette_color_waves(palette_, leds_, STRANDS_NUMBER_LEDS);
        }

        EVERY_N_MILLISECONDS(10) {
            ws2812fx_.setColor(leds_[0].r, leds_[0].g, leds_[0].b);
        }

        /*
        EVERY_N_MILLISECONDS(1000) {
            Serial.printf("ws2812fx: (%d, %d, %d)\n\r", leds_[0].r, leds_[0].g, leds_[0].b);
        }
        */
    }

    ws2812fx_.service();
}

void Ws2812fxDriver::blank(uint32_t color) {
    setMode(FX_MODE_STATIC);
    setColor(color);
    for (auto i = 0u; i < STRANDS_NUMBER_LEDS; ++i) {
        leds_[i] = color;
    }
}

void Ws2812fxDriver::start() {
    ws2812fx_.start();
}

void Ws2812fxDriver::stop() {
    ws2812fx_.stop();
}

bool Ws2812fxDriver::isRunning() {
    return ws2812fx_.isRunning();
}

CRGBPalette16 Ws2812fxDriver::getPalette() {
    return palette_;
}

void Ws2812fxDriver::setPalette(CRGBPalette16 palette) {
    // palette_ = palette;
    target_ = palette;
    palettized_ = true;
    Serial.println("ws2812fx: new palette");
}

uint8_t Ws2812fxDriver::getBrightness() {
    return ws2812fx_.getBrightness();
}

void Ws2812fxDriver::setBrightness(uint8_t value) {
    ws2812fx_.setBrightness(value);
}

uint8_t Ws2812fxDriver::getMode() {
    return ws2812fx_.getMode();
}

void Ws2812fxDriver::setMode(uint8_t value) {
    ws2812fx_.setMode(value);
}

uint16_t Ws2812fxDriver::getSpeed() {
    return ws2812fx_.getSpeed();
}

void Ws2812fxDriver::setSpeed(uint16_t value) {
    ws2812fx_.setSpeed(value);
}

uint8_t Ws2812fxDriver::getModeCount() {
    return ws2812fx_.getModeCount();
}

const __FlashStringHelper *Ws2812fxDriver::getModeName(uint8_t i) {
    return ws2812fx_.getModeName(i);
}

void Ws2812fxDriver::setColor(uint32_t value) {
    ws2812fx_.setColor(value);
    // palette_ = CRGBPalette16{ value };
    target_ = palette_;
    palettized_ = false;
    Serial.print("ws2812fx: color=");
    Serial.printf("%d\n\r", value);
}

uint16_t Ws2812fxDriver::color_waves() {
    static uint8_t palette_number_ = 0;

    EVERY_N_SECONDS(STRANDS_SECONDS_PER_PALETTE) {
        palette_number_ = addmod8(palette_number_, 1, gGradientPaletteCount);
        target_ = gGradientPalettes[palette_number_].p;
    }

    return static_waves();
}

uint16_t Ws2812fxDriver::static_waves() {
    EVERY_N_MILLISECONDS(40) {
        nblendPaletteTowardPalette(palette_, target_, 16);
    }

    EVERY_N_MILLISECONDS(20) {
        palette_color_waves(palette_, leds_, STRANDS_NUMBER_LEDS);
    }

    auto seg = ws2812fx_.getSegment();
    for (uint16_t i = seg->stop; i > seg->start; i--) {
        ws2812fx_.setPixelColor(i, leds_[i].r, leds_[i].g, leds_[i].b);
    }

    return 10;
}