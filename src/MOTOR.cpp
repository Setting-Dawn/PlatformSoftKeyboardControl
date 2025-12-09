/** Implementation of motor commands */

#include "MOTOR.h"
#include <Arduino.h>

/* initialize pins for PWM output and motor control*/
void MOTOR_init(uint8_t pin1,uint8_t pin2, uint8_t channel1, uint8_t channel2) {
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    ledcSetup(channel1, 25000, 8);
    ledcSetup(channel2, 25000, 8);
    ledcAttachPin(pin1, channel1);
    ledcAttachPin(pin2, channel2);
}
    
void MOTOR_forward(uint8_t pin1,uint8_t pin2, uint8_t channel1, uint8_t channel2, uint8_t speed) {
    ledcWrite(channel1, speed);
    ledcWrite(channel2, 0);
}

void MOTOR_reverse(uint8_t pin1,uint8_t pin2, uint8_t channel1, uint8_t channel2, uint8_t speed) {
    ledcWrite(channel1, 0);
    ledcWrite(channel2, speed);
}

void MOTOR_brake(uint8_t pin1,uint8_t pin2, uint8_t channel1, uint8_t channel2) {
    ledcWrite(channel1, 255);
    ledcWrite(channel2, 255);
}