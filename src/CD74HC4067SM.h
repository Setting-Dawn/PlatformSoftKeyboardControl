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

/**
 * @class CD74HC4067SM
 * @brief A class to control a CD74HC4067SM 1:16 analog multiplexer/demultiplexer chip.
 * 
 * This class provides a convenient interface for controlling a CD74HC4067SM multiplexer,
 * which allows selecting one of 16 analog channels using 4 binary select pins (S0-S3)
 * and an active-low enable pin. The chip is commonly used in sensor arrays or to expand
 * the number of analog inputs available on a microcontroller.
 * 
 * @details The multiplexer uses binary addressing via pins S0, S1, S2, and S3 to select
 * one of 16 channels (0-15). When enabled (nE = LOW), the selected channel is connected
 * to the common I/O pin. When disabled (nE = HIGH), no channels are connected.
 */
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