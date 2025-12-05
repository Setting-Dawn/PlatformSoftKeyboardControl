/**
 * @file IMUReader.cpp
 * @brief Simple IMU reader for ESP32 + Adafruit BNO055 (2D tilt).
 *
 * This module:
 *  - Initializes the BNO055 absolute orientation sensor over I2C.
 *  - On each call to IMUReader_getXYAngles(), reads Euler angles.
 *  - Captures a "horizontal" reference orientation once system calibration is
 *    non-zero.
 *  - Computes X/Y tilt (roll/pitch) relative to that horizontal baseline.
 *
 * No Serial printing and no FreeRTOS tasks are used.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#include "IMUReader.h"

// ---------- Configuration constants -------------------------------------------------

/**
 * @brief IMU I2C address.
 *
 * For the Adafruit BNO055 breakout:
 *  - ADR low / default: 0x28
 *  - ADR high         : 0x29
 *
 * Change this if you have modified the ADR jumper.
 */
static constexpr uint8_t BNO055_I2C_ADDRESS = 0x28;

// ---------- Internal state ----------------------------------------------------------

/** @brief BNO055 IMU object. */
static Adafruit_BNO055 g_bno(55, BNO055_I2C_ADDRESS);

/** @brief True once IMUReader_begin() has successfully initialized the IMU. */
static bool g_initialized = false;

/**
 * @brief True once a horizontal reference orientation has been captured.
 */
static bool g_haveBaseline = false;

/**
 * @brief Euler angles (heading, roll, pitch) captured as "horizontal" reference.
 *
 * Units: degrees. Axes follow Adafruit_BNO055 convention:
 *  - X: heading (yaw)
 *  - Y: roll
 *  - Z: pitch
 */
static imu::Vector<3> g_baselineEuler;

// ---------- Utility functions -------------------------------------------------------

/**
 * @brief Wrap an angle in degrees into the range [-180, 180).
 *
 * @param angleDeg Angle in degrees (any range).
 * @return Wrapped angle in degrees.
 */
static float wrapAngle180(float angleDeg)
{
    while (angleDeg >= 180.0f) {
        angleDeg -= 360.0f;
    }
    while (angleDeg < -180.0f) {
        angleDeg += 360.0f;
    }
    return angleDeg;
}

/**
 * @brief Initialize the I2C bus for the ESP32.
 *
 * Uses the default I2C pins configured by the board variant. If your hardware
 * routes the BNO055 to custom pins, replace Wire.begin() with
 * Wire.begin(sdaPin, sclPin).
 */
static void initI2CBus()
{
    Wire.begin();
}

/**
 * @brief Initialize the BNO055 IMU.
 *
 * @return true if initialization succeeded, false otherwise.
 */
static bool initIMU()
{
    if (!g_bno.begin()) {
        return false;
    }

    // Give the sensor time to boot and stabilize.
    delay(1000);

    // Use the external crystal for better accuracy (if present on breakout).
    g_bno.setExtCrystalUse(true);

    return true;
}

// ---------- Public API --------------------------------------------------------------

bool IMUReader_begin()
{
    initI2CBus();

    if (!initIMU()) {
        g_initialized = false;
        return false;
    }

    g_initialized  = true;
    g_haveBaseline = false;

    return true;
}

bool IMUReader_getXYAngles(IMUXYAngles &out)
{
    if (!g_initialized) {
        return false;
    }

    // Read Euler angles (in degrees) from the BNO055.
    imu::Vector<3> euler = g_bno.getVector(Adafruit_BNO055::VECTOR_EULER);

    // Read calibration state.
    uint8_t sysCal = 0, gyroCal = 0, accelCal = 0, magCal = 0;
    g_bno.getCalibration(&sysCal, &gyroCal, &accelCal, &magCal);

    // If we haven't captured a baseline yet and system calibration is non-zero,
    // treat the current orientation as "horizontal".
    if (!g_haveBaseline && sysCal > 0) {
        g_baselineEuler = euler;
        g_haveBaseline  = true;
    }

    float xAngle = 0.0f;
    float yAngle = 0.0f;

    if (g_haveBaseline) {
        // Roll (Euler.y) relative to baseline.
        xAngle = wrapAngle180(euler.y() - g_baselineEuler.y());
        // Pitch (Euler.z) relative to baseline.
        yAngle = wrapAngle180(euler.z() - g_baselineEuler.z());
    } else {
        // Baseline not captured yet; treat current pose as zero.
        xAngle = 0.0f;
        yAngle = 0.0f;
    }

    out.xAngle = xAngle;
    out.yAngle = yAngle;
    out.sysCal = sysCal;

    return true;
}
