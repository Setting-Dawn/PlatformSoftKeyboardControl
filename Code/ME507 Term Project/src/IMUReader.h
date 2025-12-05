#ifndef IMU_READER_H
#define IMU_READER_H

#include <stdint.h>

/**
 * @brief 2D tilt angles from the IMU.
 *
 * All angles are in degrees.
 *
 * Definitions:
 *  - xAngle: rotation about the sensor X-axis (roll), relative to horizontal
 *  - yAngle: rotation about the sensor Y-axis (pitch), relative to horizontal
 */
struct IMUXYAngles
{
    float xAngle;   ///< Roll angle [deg] from horizontal (sensor X-axis).
    float yAngle;   ///< Pitch angle [deg] from horizontal (sensor Y-axis).
    uint8_t sysCal; ///< BNO055 system calibration level (0–3).
};

/**
 * @brief Initialize the IMU reader module.
 *
 * This function:
 *  - Initializes the I2C bus.
 *  - Initializes the BNO055 and configures it for absolute orientation.
 *  - Captures no baseline yet (baseline is captured later once calibration > 0).
 *
 * Call this once, for example in setup().
 *
 * @return true on success, false if the IMU could not be initialized.
 */
bool IMUReader_begin();

/**
 * @brief Get the current X/Y tilt angles from the IMU.
 *
 * This function:
 *  - Reads the current Euler angles from the BNO055.
 *  - If a horizontal baseline has not yet been captured and system calibration
 *    is > 0, it stores the current orientation as the horizontal reference.
 *  - Computes roll/pitch (X/Y tilt) relative to that baseline.
 *
 * @param[out] out Structure that will be filled with the latest angles.
 * @return true if the IMU was initialized and data was read successfully,
 *         false if the IMU was not initialized.
 */
bool IMUReader_getXYAngles(IMUXYAngles &out);

#endif // IMU_READER_H
