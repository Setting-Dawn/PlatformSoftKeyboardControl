/** 
 * @file MotorDriver.h
 * @author Jake Rowen
 * @brief Header file for motor driver class.
 * initialize initializes the motor driver, zeros the encoder and platform.
 * setAngle sets the motor to a specific angle,
 * setSpeed sets the motor to a specific speed,
 * stop stops the motor,
 * hold holds the motor at its current position.
 * @version 1.0
 * @date 2024-06-15
 * */
 
    #ifndef MOTORDRIVER_H
    #define MOTORDRIVER_H
    
    #include <Arduino.h>
    #include "Encoder.h"

    /* MOTOR DRIVER CLASS */

    class MotorDriver {
        private:
            uint8_t motorPin1;
            uint8_t motorPin2;
            uint8_t nSleepPin;
            uint8_t faultPin;
            Encoder encoder;
            float currentAngle;
            float targetAngle;
            float currentSpeed;
            void enableMotor();
            void disableMotor();
        public:
            MotorDriver(uint8_t mPin1, uint8_t mPin2, uint8_t nSLEEP, uint8_t fPin, uint8_t encA, uint8_t encB);
            void initialize();
            void setAngle(float angle);
            void setSpeed(float speed);
            void stop();
            void hold();
    };

