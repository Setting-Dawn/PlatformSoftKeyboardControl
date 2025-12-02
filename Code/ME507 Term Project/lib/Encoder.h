/** @file Encoder.h
 * @author Jake Rowen
 * @brief Header file for motor encoder class. 
 * getCount gets the current encoder count,
 *  resetCount resets the count to zero.
 * @version 1.0
 * @date 2024-06-15
 * */

 #IFNDEF ENCODER_H
 #DEFINE ENCODER_H
 #endif // ENCODER_H
 #include <Arduino.h>
 #include "ESP32Encoder.h"

 /* ENCODER CLASS*/

    class Encoder {
        private:
            ESP32Encoder encoder;
            uint8_t pinA;
            uint8_t pinB;
        public:
            Encoder(uint8_t pA, uint8_t pB);
            void init();
            int32_t getCount();
            void resetCount();
    };
