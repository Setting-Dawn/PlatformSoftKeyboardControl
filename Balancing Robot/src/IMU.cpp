#include <Arduino.h>
#include "IMU.h"

// Global/static BNO055 instance
static Adafruit_BNO055 bno = Adafruit_BNO055(55, BNO055_ADDRESS_B);
static bool imuCalibrated = false;

// Internal helper: perform blocking auto-calibration with timeout
static void IMU_runAutoCalibration(unsigned long timeoutMs) {
    unsigned long start = millis();

    // Poll calibration until fully calibrated or timeout
    while (!bno.isFullyCalibrated()) {
        // If timeoutMs == 0, don't block at all
        if (timeoutMs > 0 && (millis() - start >= timeoutMs)) {
            break;
        }

        // Optional: you could add Serial prints here for debug, e.g.:
        // uint8_t sys, gyro, accel, mag;
        // bno.getCalibration(&sys, &gyro, &accel, &mag);
        // Serial.print("Calib: SYS=");
        // Serial.print(sys);
        // Serial.print(" G=");
        // Serial.print(gyro);
        // Serial.print(" A=");
        // Serial.print(accel);
        // Serial.print(" M=");
        // Serial.println(mag);

        delay(100);
    }

    imuCalibrated = bno.isFullyCalibrated();
}

bool IMU_init() {
    // Default: wait up to 30 seconds for calibration
    const unsigned long defaultTimeoutMs = 30000;
    return IMU_init(defaultTimeoutMs);
}

bool IMU_init(unsigned long calibrationTimeoutMs) {
    if (!bno.begin(OPERATION_MODE_NDOF)) {
        // Sensor not found
        imuCalibrated = false;
        return false;
    }

    delay(100);
    bno.setExtCrystalUse(true);

    // Run auto calibration (blocking until calibrated or timeout)
    IMU_runAutoCalibration(calibrationTimeoutMs);

    return true;  // Sensor initialized, even if not fully calibrated yet
}

void IMU_getAngles(float &x_angle, float &y_angle) {
    imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

    // BNO055 Euler format:
    //   euler.x = heading (yaw)
    //   euler.y = roll
    //   euler.z = pitch

    x_angle = euler.y();   // Roll
    y_angle = euler.z();   // Pitch
}

bool IMU_isCalibrated() {
    // Keep our cached value in sync with sensor status
    imuCalibrated = bno.isFullyCalibrated();
    return imuCalibrated;
}

