#include "ws2812fx.h"

void Ws2812fxDriver::begin() {
    ws2812fx_.init();
    ws2812fx_.setMode(FX_MODE_STATIC);
    ws2812fx_.setMode(FX_MODE_RUNNING_RANDOM2);
    ws2812fx_.setMode(FX_MODE_MERRY_CHRISTMAS);
    ws2812fx_.setBrightness(STRANDS_BRIGHTNESS);
    ws2812fx_.setColor(0xFF5900);
    ws2812fx_.setSpeed(1000);
    ws2812fx_.start();
}

void Ws2812fxDriver::service(bool enabled) {
    if (!enabled) {
        blank(0);
    }
    ws2812fx_.service();
}

void Ws2812fxDriver::blank(uint32_t color) {
    ws2812fx_.setMode(FX_MODE_STATIC);
    ws2812fx_.setColor(color);
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
}