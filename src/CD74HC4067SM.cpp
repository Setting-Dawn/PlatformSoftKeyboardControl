/*!
 * @file CD74HC4067SM.cpp
 * @author Setting-Dawn
 * @brief Implementation of CD74HC4067SM multiplexer class.
 * @version 1.0.0
 * @date 2025-NOV-12
 */

#include "CD74HC4067SM.h"

/**
 * @class CD74HC4067SM
 * @brief A class to control a CD74HC4067SM 1:16 analog multiplexer/demultiplexer chip.
 * 
 * This class provides a convenient interface for controlling a CD74HC4067SM multiplexer,
 * which allows selecting one of 16 analog channels using 4 binary select pins (S0-S3)
 * and an active-low enable pin. The chip is commonly used in sensor arrays or to expand
 * the number of analog inputs available on a microcontroller.
 * 
* @param Spin0 the Arduino pin used to control the s0 pin
* @param Spin1 the Arduino pin used to control the s1 pin
* @param Spin2 the Arduino pin used to control the s2 pin
* @param Spin3 the Arduino pin used to control the s3 pin
* @param enablePin the Arduino pin used to control !E pin
 * 
 * @details The multiplexer uses binary addressing via pins S0, S1, S2, and S3 to select
 * one of 16 channels (0-15). When enabled (nE = LOW), the selected channel is connected
 * to the common I/O pin. When disabled (nE = HIGH), no channels are connected.
 * 
 * @see switchPin(), enable(), disable()
 */
CD74HC4067SM::CD74HC4067SM(uint8_t SPin0,
                            uint8_t SPin1,
                            uint8_t SPin2,
                            uint8_t SPin3,
                            uint8_t enablePin) 
{
    // Assign all pins in use
    s0 = SPin0;
    s1 = SPin1;
    s2 = SPin2;
    s3 = SPin3;
    nE = enablePin;

    // Assign Arduino pins to OUTPUT mode
    pinMode(s0,OUTPUT);
    pinMode(s1,OUTPUT);
    pinMode(s2,OUTPUT);
    pinMode(s3,OUTPUT);
    pinMode(nE,OUTPUT);
};

/*! @brief switches connection to chosen pin value
* @details enables the set of s-pins to represent the binary
* of the chosen pin which closes the selected circuit to the chip output
* @param pin the 0-15 value of the desired channel
*/
void CD74HC4067SM::switchPin(uint8_t pin) 
{
    // s0 Controls bit 0, check if the requested pin
    // needs s0 to be HIGH, otherwise set it LOW
    if (pin & 0001) {digitalWrite(s0,HIGH);}
    else {digitalWrite(s0,LOW);}

    // s1 Controls bit 1, check if the requested pin
    // needs s1 to be HIGH, otherwise set it LOW
    if (pin & 0010) {digitalWrite(s1,HIGH);}
    else { digitalWrite(s1,LOW);}

    // s2 Controls bit 2, check if the requested pin
    // needs s2 to be HIGH, otherwise set it LOW
    if (pin & 0100) {digitalWrite(s2,HIGH);}
    else {digitalWrite(s2,LOW);}

    // s3 Controls bit 3, check if the requested pin 
    // needs s3 to be HIGH, otherwise set it LOW
    if (pin & 1000) {digitalWrite(s3,HIGH);}
    else {digitalWrite(s3,LOW);}
}

/*! @brief Turns on pass-through
* @details sets the notEnable pin LOW to enable the use
* of the selected channel
* @param void no input is used
*/
void CD74HC4067SM::enable(void) {digitalWrite(nE,LOW);}

/*! @brief Turns off pass-through
* @details sets the notEnable pin HIGH to set to no channels
* @param void no input is used
*/
void CD74HC4067SM::disable(void) {digitalWrite(nE,HIGH);}