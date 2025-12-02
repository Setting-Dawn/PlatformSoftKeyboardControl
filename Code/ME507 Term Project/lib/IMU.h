/**
 * @file IMU.h
 * @author Jake Rowen
 * @brief Header file for IMU (Inertial Measurement Unit) class.
 * Handles initialization and data retrieval from the BNO055 sensor.
 * @version 1.0
 * @date 2024-06-15
 * 
 */

#ifndef IMU_H
#define IMU_H
#include <Arduino.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
/* IMU CLASS */

class IMU {
    private:
        Adafruit_BNO055 bno;
    public:
        IMU(uint8_t id = 55, uint8_t address = 0x28);
        bool begin();
        imu::Vector<3> getEuler();
        imu::Vector<3> getLinearAccel();
};
