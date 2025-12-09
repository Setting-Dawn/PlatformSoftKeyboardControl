/** Implementation file for encoders */
#include "ENCODER.h"

// Initialize encoders
void ENCODER_init(ESP32Encoder &encoder, uint8_t pinA, uint8_t pinB, bool inverted) {
    ESP32Encoder::useInternalWeakPullResistors = puType ::up;
    encoder.attachFullQuad(pinA, pinB);
    if (inverted) {
        encoder.setCount(-encoder.getCount());
    }
}

// Get encoder count
int64_t ENCODER_getCount(ESP32Encoder &encoder) {
    return encoder.getCount();
}

// Reset encoder count to zero
void ENCODER_zero(ESP32Encoder &encoder) {
    encoder.setCount(0);
}
