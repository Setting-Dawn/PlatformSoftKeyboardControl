/** Header file for motor commands */
#ifndef MOTOR_H
#define MOTOR_H
#endif

#include <Arduino.h>
// Initialize motors (e.g., set pin modes)
void MOTOR_init(uint8_t pin1, uint8_t pin2, uint8_t channel1, uint8_t channel2);
void MOTOR_forward(uint8_t pin1, uint8_t pin2, uint8_t channel1, uint8_t channel2, uint8_t speed);
void MOTOR_reverse(uint8_t pin1, uint8_t pin2, uint8_t channel1, uint8_t channel2, uint8_t speed);
void MOTOR_brake(uint8_t pin1, uint8_t pin2, uint8_t channel1, uint8_t channel2);