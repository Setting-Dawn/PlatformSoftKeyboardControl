/** 
 * @file MotorDriver.h
 * @author Jake Rowen
 * @brief Header file for motor driver class.
 * Controls motor movement including direction and speed.
 * @version 1.0
 * @date 2024-06-15
 */

#include "MotorDriver.h"
#include <Arduino.h>

/* MOTOR DRIVER CLASS IMPLEMENTATION */

MotorDriver::MotorDriver(uint8_t mPin1, uint8_t mPin2, uint8_t nSLEEP, uint8_t fPin, uint8_t encA, uint8_t encB) 
    : motorPin1(mPin1), motorPin2(mPin2), nSleepPin(nSLEEP), faultPin(fPin), encoder(encA, encB), currentAngle(0.0), targetAngle(0.0), currentSpeed(0.0) {}
/* Enables motor by setting nSleep pin HIGH */
    void MotorDriver::enableMotor() {
    digitalWrite(nSleepPin, HIGH);
}
/* Disables motor by setting nSleep pin LOW */
void MotorDriver::disableMotor() {
    digitalWrite(nSleepPin, LOW);
}
/* Takes no input and initializes motor pins and encoder */
void MotorDriver::initialize() {
    pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);
    pinMode(nSleepPin, OUTPUT);
    pinMode(faultPin, INPUT);
    disableMotor();
    encoder.init();
    encoder.resetCount();
    currentAngle = 0.0;
    targetAngle = 0.0;
    currentSpeed = 0.0;
}

/* Sets motor to a specific angle */
void MotorDriver::setAngle(float angle) {
    targetAngle = angle;
    // Implementation to move motor to targetAngle
    enableMotor();
    // Placeholder logic for moving to angle
    // Actual implementation would involve PID control or similar
}

/* Sets motor to a specific speed */
void MotorDriver::setSpeed(float speed) {
    currentSpeed = speed;
    // Implementation to set motor speed
    enableMotor();
    // Placeholder logic for setting speed
    // Actual implementation would involve PWM control or similar
}

/* Stops the motor */
void MotorDriver::stop() {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    disableMotor();
}

/* Holds the motor at its current position */
void MotorDriver::hold() {
    // Implementation to hold motor position
    enableMotor();
    // Placeholder logic for holding position
    // Actual implementation would involve maintaining current angle
}
