/*!
 * @file CD74HC4067SM.h
 * @author Setting-Dawn
 * @brief Header file for CD74HC4067SM multiplexer class.
 * @version 1.0.0
 * @date 2025-NOV-12
 */

#ifndef __CD74HC4067SM_H__
#define __CD74HC4067SM_H__

#include <Arduino.h>

class CD74HC4067SM {
    private:
        uint8_t s0;
        uint8_t s1;
        uint8_t s2;
        uint8_t s3;
        uint8_t nE;
    public:
        CD74HC4067SM(uint8_t SPin0,
            uint8_t SPin1,
            uint8_t SPin2,
            uint8_t SPin3,
            uint8_t enablePin);
        void switchPin(uint8_t pin);
        void enable(void);
        void disable(void);
};

#endif //__CD74HC4067SM_H__