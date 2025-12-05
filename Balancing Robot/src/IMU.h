#ifndef IMU_H
#define IMU_H

#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// Initialize IMU with default calibration timeout (ms).
// Returns true if sensor is found and initialized (even if not fully calibrated yet).
bool IMU_init();

// Initialize IMU with a specific calibration timeout (ms).
// Pass 0 to skip blocking for calibration.
bool IMU_init(unsigned long calibrationTimeoutMs);

// Get roll (X) and pitch (Y) in degrees.
void IMU_getAngles(float &x_angle, float &y_angle);

// Check if the BNO055 reports "fully calibrated".
bool IMU_isCalibrated();

#endif
