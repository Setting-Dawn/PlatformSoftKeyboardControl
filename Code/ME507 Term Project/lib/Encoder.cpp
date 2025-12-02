/**
 * @file  Encoder.cpp
 * @author Jake Rowen
 * @brief Implementation file for motor encoder class.
 * @version 1.0
 * @date 2024-06-15
 */

#include "Encoder.h"
#include <Arduino.h>
#include "ESP32Encoder.h"

/* ENCODER CLASS IMPLEMENTATION */

Encoder::Encoder(uint8_t pA, uint8_t pB) : pinA(pA), pinB(pB) {}
void Encoder::init() {
    ESP32Encoder::useInternalWeakPullResistors = UP;
    encoder.attachFullQuad(pinA, pinB);
    encoder.clearCount();
}
int32_t Encoder::getCount() {
    return encoder.getCount();
}
void Encoder::resetCount() {
    encoder.clearCount();
}

