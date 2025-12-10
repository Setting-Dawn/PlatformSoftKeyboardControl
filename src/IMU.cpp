/**
 * @file IMU.cpp
 * @brief Implementation of IMU helper functions for the BNO055 sensor.
 * @details Implements initialization, calibration helper, angle reading,
 *          and calibration status checks for the Adafruit BNO055 IMU used
 *          by the platform. Add `EXTRACT_STATIC = YES` to your Doxygen
 *          configuration if you want static/internal functions to be
 *          included in the generated documentation (e.g. IMU_runAutoCalibration).
 */
#include <Arduino.h>
#include "IMU.h"

// Global/static BNO055 instance
static Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
static bool imuCalibrated = false;

/**
 * @brief Internal helper function that performs blocking auto-calibration with timeout.
 * 
 * Continuously polls the BNO055 sensor until it reports fully calibrated or the
 * timeout period expires. Updates the global imuCalibrated flag based on the result.
 * 
 * @param timeoutMs Timeout duration in milliseconds. If 0, skips blocking entirely.
 * @return void
 * 
 * @details This is an internal helper function. If timeoutMs is 0, the function
 * returns immediately without waiting. Otherwise, it blocks and polls every 100ms
 * until the sensor is fully calibrated or the timeout is reached.
 */
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

/**
 * @brief Initialize the IMU sensor with default calibration timeout.
 * 
 * This is a convenience overload that initializes the BNO055A IMU sensor with
 * a default calibration timeout of 30 seconds.
 * 
 * @return bool True if the sensor is found and successfully initialized (even if
 *         not fully calibrated yet), false if sensor initialization fails.
 * 
 * @see IMU_init(unsigned long calibrationTimeoutMs)
 */
bool IMU_init() {
    // Default: wait up to 30 seconds for calibration
    const unsigned long defaultTimeoutMs = 30000;
    return IMU_init(defaultTimeoutMs);
}

/**
 * @brief Initialize the IMU sensor with a specific calibration timeout.
 * 
 * Initializes the BNO055 IMU sensor in NDOF (Nine Degrees of Freedom) operation
 * mode, configures the external crystal, and runs auto-calibration with the
 * specified timeout duration.
 * 
 * @param calibrationTimeoutMs Timeout duration for calibration in milliseconds.
 *                              Pass 0 to skip blocking for calibration.
 * 
 * @return bool True if the sensor is found and successfully initialized (even if
 *         not fully calibrated yet), false if sensor initialization fails.
 * 
 * @details The function performs the following steps:
 *   1. Initializes the BNO055 in NDOF mode with I2C address 0x28
 *   2. Waits 100ms for stabilization
 *   3. Enables external crystal use for better accuracy
 *   4. Runs auto-calibration with the specified timeout
 */
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

/**
 * @brief Retrieve the current roll and pitch angles from the IMU.
 * 
 * Reads the Euler angles from the BNO055 sensor and extracts the roll (X)
 * and pitch (Y) components, ignoring the heading/yaw value.
 * 
 * @param[out] x_angle Reference to a float that will be filled with the roll angle
 *                      in degrees.
 * @param[out] y_angle Reference to a float that will be filled with the pitch angle
 *                      in degrees.
 * 
 * @return void
 * 
 * @details The BNO055 Euler vector components are mapped as follows:
 *   - euler.x = heading/yaw (not returned)
 *   - euler.y = roll (output as x_angle)
 *   - euler.z = pitch (output as y_angle)
 * 
 * @note Ensure IMU_init() has been called before using this function.
 */
void IMU_getAngles(float &x_angle, float &y_angle) {
    imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

    // BNO055 Euler format:
    //   euler.x = heading (yaw)
    //   euler.y = roll
    //   euler.z = pitch

    x_angle = euler.y();   // Roll
    y_angle = euler.z();   // Pitch
}

/**
 * @brief Check if the BNO055A IMU is fully calibrated.
 * 
 * Queries the sensor to determine if it has achieved full calibration status for
 * all nine degrees of freedom (system, gyroscope, accelerometer, and magnetometer).
 * Updates the internal calibration cache before returning.
 * 
 * @return bool True if the sensor reports full calibration, false otherwise.
 * 
 * @details The function updates the global imuCalibrated cache variable with the
 * current sensor status to keep it synchronized with the actual hardware state.
 * 
 * @note Full calibration typically requires the sensor to be in motion, exposed
 * to the Earth's magnetic field, and given sufficient time to converge.
 */
bool IMU_isCalibrated() {
    // Keep our cached value in sync with sensor status
    imuCalibrated = bno.isFullyCalibrated();
    return imuCalibrated;
}

