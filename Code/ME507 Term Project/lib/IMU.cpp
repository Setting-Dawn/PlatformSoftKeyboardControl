/** 
 * @file IMU.cpp
 * @author Jake Rowen
 * @brief Implementation file for IMU (Inertial Measurement Unit) class.
 * Handles initialization and data retrieval from the BNO055 sensor.
 * @version 1.0
 * @date 2024-06-15
 */

#include "IMU.h"
/* IMU CLASS IMPLEMENTATION */

IMU::IMU(uint8_t id, uint8_t address) : bno(id, address) {}
bool IMU::begin() {
    return bno.begin();
}
imu::Vector<3> IMU::getEuler() {
    sensors_event_t event; 
    bno.getEvent(&event, Adafruit_BNO055::VECTOR_EULER);
    return imu::Vector<3>(event.orientation.x, event.orientation.y, event.orientation.z);
}
imu::Vector<3> IMU::getLinearAccel() {
    sensors_event_t event; 
    bno.getEvent(&event, Adafruit_BNO055::VECTOR_LINEARACCEL);
    return imu::Vector<3>(event.acceleration.x, event.acceleration.y, event.acceleration.z);
}
