/** Encoder header file */

#ifndef ENCODER_H
#define ENCODER_H
#endif
#include <ESP32Encoder.h>
#include <Arduino.h>

// Initialize encoders
void ENCODER_init(ESP32Encoder &encoder, uint8_t pinA, uint8_t pinB, bool inverted = false);
// Get encoder count
int64_t ENCODER_getCount(ESP32Encoder &encoder);
// Reset encoder count to zero
void ENCODER_zero(ESP32Encoder &encoder);
