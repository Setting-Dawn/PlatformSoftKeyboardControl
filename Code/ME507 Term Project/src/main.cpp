
#include <Wire.h>
#include <utility>
#include "ADC128D818.h"
#include "CD74HC4067SM.h"
#include "PCA9956.h"
#include <ESP32Encoder.h>
#include <IMU.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"

// Multiplexer control pins
const uint8_t s0_PIN          = 26;
const uint8_t s1_PIN          = 25;
const uint8_t s2_PIN          = 33;
const uint8_t s3_PIN          = 32;
// WARNING: 35 is input-only on most ESP32s – change if this needs to be an output
const uint8_t MultiEnable_PIN = 35;

// ADC I2C Addresses
const uint8_t ADC_1ADDRESS    = 0x1D;
const uint8_t ADC_2ADDRESS    = 0x1F;

// PCA9956BTWY Address
const uint8_t PCA9956_ADDRESS = 0x01;

// Motor control pins (double-check your board's pinout)
const uint8_t FAULT_PIN  = 4;
const uint8_t NSLEEP_PIN = 2;
const uint8_t MOTOR_X_1  = 31;
const uint8_t MOTOR_X_2  = 30;
const uint8_t MOTOR_Y_1  = 28;
const uint8_t MOTOR_Y_2  = 27;

// Global hardware objects
ADC128D818   ADC_1(ADC_1ADDRESS);
ADC128D818   ADC_2(ADC_2ADDRESS);
CD74HC4067SM Multiplex(s0_PIN, s1_PIN, s2_PIN, s3_PIN, MultiEnable_PIN);
PCA9956      CurrCtrl(&Wire);
ESP32Encoder encoderX;
ESP32Encoder encoderY;

void task_ReadMaterial(void* p_params) {
    CurrCtrl.init(PCA9956_ADDRESS, 0xFF); // Initialize current control address and max brightness
    
    // TODO: ADC_1 / ADC_2 init, Multiplex setup, etc.
    
    while (true) {
        // TODO: read sensors, print, etc.
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


}

// ---------- Configuration constants -------------------------------------------------

/** @brief Serial baud rate for debug and data output. */
static constexpr uint32_t SERIAL_BAUD_RATE = 115200;

/**
 * @brief Desired IMU task period in milliseconds.
 * @details 20 ms ≈ 50 Hz, well within the BNO055 100 Hz orientation rate.
 */
static constexpr uint32_t IMU_TASK_PERIOD_MS = 20;

/** @brief Stack size for the IMU task (in 32-bit words). */
static constexpr uint32_t IMU_TASK_STACK_SIZE = 4096;

/** @brief FreeRTOS priority for the IMU task. */
static constexpr UBaseType_t IMU_TASK_PRIORITY = 1;

/** @brief Core on which to pin the IMU task (0 or 1 for ESP32). */
static constexpr BaseType_t IMU_TASK_CORE = 1;

// ---------- Global objects & state --------------------------------------------------

/**
 * @brief BNO055 IMU object.
 *
 * According to the schematic, ADR is pulled high, so the device address is 0x70.
 */
Adafruit_BNO055 g_bno(55, 0x28);

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
static imu::Vector<3> g_baselineEuler;  // Initialized when g_haveBaseline becomes true.

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
 * Uses the default NodeMCU I2C pins as wired in the schematic
 * (SDA/SCL from the module header).
 */
static void initI2CBus()
{
    // Use default SDA/SCL pins configured by the board variant.
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
        Serial.println(F("ERROR: BNO055 not detected. Check wiring and I2C address (0x29)."));
        return false;
    }

    // Give the sensor time to boot and stabilize.
    delay(1000);

    // Use the external crystal for better accuracy (if present on breakout).
    g_bno.setExtCrystalUse(true);

    Serial.println(F("BNO055 initialized successfully."));
    return true;
}

/**
 * @brief Print current calibration status for debugging.
 *
 * System calibration should ideally be > 0 for good absolute orientation.
 * For relative-from-horizontal measurements, full calibration is helpful
 * but not strictly required, depending on your tolerance for drift.
 */
static void printCalibrationStatus()
{
    uint8_t sys = 0, gyro = 0, accel = 0, mag = 0;
    g_bno.getCalibration(&sys, &gyro, &accel, &mag);

    Serial.print(F("CAL (sys, gyro, accel, mag): "));
    Serial.print(sys);
    Serial.print(F(", "));
    Serial.print(gyro);
    Serial.print(F(", "));
    Serial.print(accel);
    Serial.print(F(", "));
    Serial.println(mag);
}

/**
 * @brief Read Euler angles (heading, roll, pitch) from the IMU.
 *
 * @param[out] eulerOut Output vector containing Euler angles in degrees.
 * @param[out] sysCalOut Output system calibration level (0–3).
 * @return true if the read was successful.
 */
static bool readEulerAngles(imu::Vector<3> &eulerOut, uint8_t &sysCalOut)
{
    // Read Euler angles (in degrees) from the BNO055.
    eulerOut = g_bno.getVector(Adafruit_BNO055::VECTOR_EULER);

    // Also read calibration state.
    uint8_t gyro = 0, accel = 0, mag = 0;
    g_bno.getCalibration(&sysCalOut, &gyro, &accel, &mag);

    // Add any additional sanity checks here if desired.
    return true;
}

/**
 * @brief Capture the current Euler orientation as the "horizontal" reference.
 *
 * This sets the baseline so that future readings can be expressed as
 * angles relative to this pose.
 *
 * @param currentEuler Current Euler angles in degrees.
 */
static void setHorizontalBaseline(const imu::Vector<3> &currentEuler)
{
    g_baselineEuler = currentEuler;
    g_haveBaseline  = true;

    Serial.println(F("Horizontal reference captured (baseline updated)."));
}

/**
 * @brief Handle simple serial commands.
 *
 * Current commands:
 *  - 'z' or 'Z' : Re-zero the horizontal reference using the latest IMU orientation.
 *
 * This function is non-blocking and should be called frequently in the IMU task.
 *
 * @param latestEuler Latest Euler readings to use if re-zeroing.
 */
static void handleSerialCommands(const imu::Vector<3> &latestEuler)
{
    while (Serial.available() > 0) {
        const int c = Serial.read();
        if (c == 'z' || c == 'Z') {
            setHorizontalBaseline(latestEuler);
        }
    }
}

/**
 * @brief Print orientation data (absolute and relative to horizontal).
 *
 * @param eulerEuler Euler angles (heading, roll, pitch) in degrees.
 */
static void printOrientation(const imu::Vector<3> &eulerEuler)
{
    // Absolute orientation (as reported by IMU)
    const float heading = eulerEuler.x(); // yaw
    const float roll    = eulerEuler.y();
    const float pitch   = eulerEuler.z();

    // Relative to "horizontal" baseline, if available
    float relHeading = 0.0f;
    float relRoll    = 0.0f;
    float relPitch   = 0.0f;

    if (g_haveBaseline) {
        relHeading = wrapAngle180(heading - g_baselineEuler.x());
        relRoll    = wrapAngle180(roll    - g_baselineEuler.y());
        relPitch   = wrapAngle180(pitch   - g_baselineEuler.z());
    }

    Serial.print(F("ABS  [deg]  H: "));
    Serial.print(heading, 2);
    Serial.print(F("  R: "));
    Serial.print(roll, 2);
    Serial.print(F("  P: "));
    Serial.print(pitch, 2);

    Serial.print(F("   |   REL  [deg]  dH: "));
    Serial.print(relHeading, 2);
    Serial.print(F("  dR: "));
    Serial.print(relRoll, 2);
    Serial.print(F("  dP: "));
    Serial.print(relPitch, 2);
    Serial.println();
}

// ---------- FreeRTOS task -----------------------------------------------------------

/**
 * @brief FreeRTOS task that periodically reads the IMU and outputs orientation.
 *
 * This task:
 *  - Runs at a fixed period (IMU_TASK_PERIOD_MS).
 *  - Reads Euler angles (heading, roll, pitch) from the BNO055.
 *  - Captures a horizontal baseline once some system calibration is present.
 *  - Handles serial commands (e.g., 'z' to re-zero).
 *  - Prints absolute and relative angles to Serial.
 *
 * @param pvParameters Unused task parameter.
 */
static void taskIMU(void *pvParameters)
{
    (void)pvParameters;

    TickType_t lastWakeTime = xTaskGetTickCount();

    imu::Vector<3> euler;
    uint8_t sysCal = 0;

    for (;;)
    {
        // Wait until the next cycle.
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(IMU_TASK_PERIOD_MS));

        // Read current IMU orientation and system calibration.
        if (!readEulerAngles(euler, sysCal)) {
            Serial.println(F("WARNING: Failed to read Euler angles from BNO055."));
            continue;
        }

        // If we haven't captured a baseline yet, do so once we have some system calibration.
        if (!g_haveBaseline && sysCal > 0) {
            setHorizontalBaseline(euler);
        }

        // Handle serial commands (e.g., 'z' to re-zero).
        handleSerialCommands(euler);

        // Optional: Uncomment to periodically print calibration status.
        // printCalibrationStatus();

        // Print orientation data.
        printOrientation(euler);
    }
}

// ---------- Arduino core functions --------------------------------------------------

/**
 * @brief Arduino setup function.
 *
 * Initializes Serial, I2C, and the BNO055 IMU, then creates the IMU FreeRTOS task.
 * The system will halt if the IMU cannot be detected.
 */

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(1000);
    Serial.println("Hello World!");

     Serial.println();
    Serial.println(F("ESP32 + BNO055 Orientation Reader (FreeRTOS Task)"));
    Serial.println(F(" - Place system in desired horizontal pose to set baseline"));
    Serial.println(F(" - Send 'z' over serial to re-zero horizontal at any time"));
    Serial.println();

    initI2CBus();

    if (!initIMU()) {
        // Fatal error: IMU not found. Stop here.
        while (true) {
            delay(1000);
        }
    }

    printCalibrationStatus();

    // Create the IMU task pinned to a specific core.
    xTaskCreatePinnedToCore(
        taskIMU,                 ///< Task function.
        "IMU_Task",              ///< Name (for debugging).
        IMU_TASK_STACK_SIZE,     ///< Stack size in words.
        nullptr,                 ///< Task parameter.
        IMU_TASK_PRIORITY,       ///< Task priority.
        nullptr,                 ///< Task handle (not used).
        IMU_TASK_CORE            ///< Core ID.
    );
}

void loop() {
    // Empty. Tasks are running independently.
}

}
