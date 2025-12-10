/*! 
 * @file ENCODER.cpp
 * @brief Implementation file for encoder helper functions.
 * @details Provides initialization and simple helper wrappers for the
 *          ESP32Encoder objects used by the project.
 */
#include "ENCODER.h"

/**
 * @brief Initialize an encoder with full quadrature decoding.
 * 
 * Configures the ESP32 encoder to use internal weak pull-up resistors and
 * attaches it to the specified pins for full quadrature counting. If inverted
 * flag is set, the encoder count direction is reversed.
 * 
 * @param encoder Reference to the ESP32Encoder object to initialize
 * @param pinA First GPIO pin for encoder input (typically Phase A)
 * @param pinB Second GPIO pin for encoder input (typically Phase B)
 * @param inverted Boolean flag to reverse the count direction:
 *                 true = negate the current count for inverted direction
 *                 false = use normal counting direction
 * 
 * @return void
 * 
 * @details Enables ESP32Encoder internal weak pull-up resistors and uses
 * full quadrature decoding for 4x count resolution.
 * 
 * @see ENCODER_getCount(), ENCODER_zero()
 */
void ENCODER_init(ESP32Encoder &encoder, uint8_t pinA, uint8_t pinB, bool inverted) {
    ESP32Encoder::useInternalWeakPullResistors = puType ::up;
    encoder.attachFullQuad(pinA, pinB);
    if (inverted) {
        encoder.setCount(-encoder.getCount());
    }
}

/**
 * @brief Retrieve the current encoder count value.
 * 
 * Returns the current count from the ESP32 encoder. The count represents
 * the accumulated encoder position and may be positive or negative depending
 * on the direction of rotation and whether the encoder was inverted.
 * 
 * @param encoder Reference to the ESP32Encoder object to read from
 * 
 * @return int64_t The current encoder count value
 * 
 * @details This function reads the counter without modifying its state.
 * For full quadrature decoding, each mechanical detent may count as multiple
 * counts depending on the encoder type (typically 4 counts per detent).
 * 
 * @see ENCODER_init(), ENCODER_zero()
 */
int64_t ENCODER_getCount(ESP32Encoder &encoder) {
    return encoder.getCount();
}

/**
 * @brief Reset the encoder count to zero.
 * 
 * Sets the encoder's internal counter to 0, allowing measurement of displacement
 * from this new reference point. This is useful for tracking relative motion
 * or establishing a home position.
 * 
 * @param encoder Reference to the ESP32Encoder object to reset
 * 
 * @return void
 * 
 * @see ENCODER_init(), ENCODER_getCount()
 */
void ENCODER_zero(ESP32Encoder &encoder) {
    encoder.setCount(0);
}
