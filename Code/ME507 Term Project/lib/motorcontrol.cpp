/**
 * @file motorcontrol.cpp
 * @author Jake Rowen
 * @brief Implementation file for motor control functions and definitions.
 * @version 1.0
 * @date 2024-06-15
 */

#include "motorcontrol.h"
// Motor Class Implementation

Motor::Motor(uint8_t p1, uint8_t p2) : pin1(p1), pin2(p2), speed(0) {}
void Motor::init() {
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    stop();
}
int Motor::getSpeed() {
    return speed;
}
void Motor::stop() {
    speed = 0;
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
}
int Motor::setSpeed(int spd) {
    speed = spd;
    if (speed > 0) {
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, LOW);
    } else if (speed < 0) {
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH);
    } else {
        stop();
    }
    return speed;
}


