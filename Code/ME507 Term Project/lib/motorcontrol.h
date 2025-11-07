/**
 * @file motorcontrol.h
 * @author Jake Rowen
 * @brief Header file for motor class.
 * @version 1.0
 * @date 2024-06-15
 */

#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H
#endif // MOTORCONTROL_H
#include <Arduino.h>


// Motor Class

class Motor {
    private:
        uint8_t pin1;
        uint8_t pin2;
        int speed; // Speed value from -255 to 255
    public:
        Motor(uint8_t p1, uint8_t p2);
        void init();
        int getSpeed();
        void stop();
        int setSpeed(int spd);
};







