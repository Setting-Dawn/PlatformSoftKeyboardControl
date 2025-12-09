/*! @file main.cpp
*   This project was created to run the soft-sensor activated balancing platform
*   as the final project of Cal Poly SLO Fall ME 507 class.
*   @author Jake
*   @author Setting-Dawn
*   @author Ethan Siahpush
*   @version 2.0.0
*   @date 2025-Dec-09
*/

#include <Arduino.h>
#include <ESP32Encoder.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <cmath>
#include <WebServer.h>

#include "ADC128D818.h"
#include "PCA9956.h"
#include "PrintStream.h"

#include "IMU.h"
#include "MOTOR.h"
#include "ENCODER.h"
#include "EITwebhost.h"
#include "CD74HC4067SM.h"
#include "shares.h"

#undef DEBUG_MOTOR
#undef DEBUG_READMATERIAL
#undef DEBUG_ALL
#undef DEBUG_WEB
#undef SCANTWI

#ifdef DEBUG_ALL
#define DEBUG_MOTOR
#define DEBUG_READMATERIAL
#define DEBUG_WEB
#endif

// Multiplexer control pins
const uint8_t s0_PIN = 26;
const uint8_t s1_PIN = 25;
const uint8_t s2_PIN = 33;
const uint8_t s3_PIN = 32;
const uint8_t MultiEnable_PIN = 35;

// ADC TWI Addresses
const uint8_t ADC_1ADDRESS = 0x1D;
const uint8_t ADC_2ADDRESS = 0x1F;

// PCA9956BTWY Address
const uint8_t PCA9956_ADDRESS = 0x01;
PCA9956 CurrCtrl (&Wire);

// Pin definition for motors
uint8_t nSleepPin = 02;

uint8_t motorXPin1 = 19;
uint8_t motorXPin2 = 18;

uint8_t motorYPin1 = 17;
uint8_t motorYPin2 = 16;
ESP32Encoder encoderX;
ESP32Encoder encoderY;

// A share which holds whether the external program needs to initialize
Share<bool> initializeVFLG ("Measure V0");
// A share which holds whether the external program should read voltages
Share<bool> readVFLG ("Read V");
// Shares to communicate target position between webpage and motor control tasks
Share<float> xBar ("X Centroid");
Share<float> yBar ("Y Centroid");
// A share which holds the data to be published
float publish[208] = {0};
Share<bool> dataAvailable ("Publish Flag");
// Mutex to thread protect the
SemaphoreHandle_t twiMutex;

/*!
* @brief Task to handle cycling through the pins to take EIT voltage readings
* @details First waits for initialization of twi communication to complete.
* Then each round has one channel with an applied current, one grounded, and 14 others.
* ADCs read all adc channels, discards the two non-read channels, finds the deltaV between appropriate pins
* and stores all values in the resulting array into a global array after lowering the dataAvailable flag.
* @param p_params void*, unused.
*/
void task_ReadMaterial(void* p_params) {
    Serial << "Starting Read Material Task" << endl;
    ADC128D818 ADC_1 (ADC_1ADDRESS);
    ADC128D818 ADC_2 (ADC_2ADDRESS);
    CD74HC4067SM Multiplex (s0_PIN,s1_PIN,s2_PIN,s3_PIN,MultiEnable_PIN);
    PCA9956 CurrCtrl (&Wire);

    const uint8_t maxCurrent = 0xFF;

    uint8_t currPinIndex;
    double measure[208] = {0}; // Somewhere to store the data for 1 complete measurement
    double cycleVals[16]; // Somewhere to store the data for 1 energization state
    double skipCycleVals[14]; // Somewhere to store the important data begining at the correct index for 1 energization state
    Serial << "Finished initializing Read Material Task" << endl;

    uint8_t cycleIndex = 0;
    uint8_t state = 0; // Initialization State

    for (;;) {
        #ifdef DEBUG_READMATERIAL
        Serial << "Material Task " << state << endl;
        #endif
        // Operation requires the TWI devices be initialized, but other tasks include other twi devices.
        // Priority for this isn't important, so it will just sit in this state until able to initialize.
        if (state == 0) // Start TWI communication
        {
            if (xSemaphoreTake(twiMutex,5) == pdTRUE) // Takes the mutex and returns true if successful
            {
                ADC_1.setOperationMode(SINGLE_ENDED); //Mode 1 reads voltages on channels 0-7
                ADC_1.begin(); // Initializes ADC1
                ADC_2.setOperationMode(SINGLE_ENDED);//Mode 1 reads voltages on channels 0-7
                ADC_2.begin(); // Initializes ADC2
                CurrCtrl.init(PCA9956_ADDRESS,0xFF,false); // Initialize current control address and max brightnes
                xSemaphoreGive(twiMutex); // Use of twi is complete, free mutex
                state = 1; // Switch to reading the material
            }
        }

        else if (state == 1) // Read Material
        {
            // cycleIndex initializes to n=0, which is also the grounded electrode.
            currPinIndex = (cycleIndex + 1) % 16; // The electrode with current applied is always n+1
            
            // Attempt to send TWI instruction to change LED
            if (xSemaphoreTake(twiMutex,5) == pdTRUE) // Takes the mutex and returns true if successful
            {
                CurrCtrl.offLED(cycleIndex); // Stop producing current for what will become the grounded pin
                Multiplex.switchPin(cycleIndex); // Use multiplexer to ground the appropriate pin
                CurrCtrl.onLED(currPinIndex); // Start producing current on the appropriate pin
                xSemaphoreGive(twiMutex); // Use of twi is complete, free mutex
                state = 2; // Only advances to start recording if communication was successful
            }
        }
        
        // After a short delay, try to take all the available measurements for 1 energization state
        else if (state == 2) 
        {
            if (xSemaphoreTake(twiMutex,5) == pdTRUE) // Takes the mutex and returns true if successful
            {
                // Record all ADC channels, electrodes 0-15 are recorded in order by ADC1 7-0 and then ADC2 7-0
                for (uint8_t i=0;i<8;i++) {
                    cycleVals[i] = ADC_1.readConverted(7-i);
                    cycleVals[8+i] = ADC_2.readConverted(7-i);

                    #ifdef DEBUG_READMATERIAL
                    Serial << cycleVals[i] << endl;
                    Serial << cycleVals[8+i] << endl;
                    #endif

                };
                Serial << "Finished a full round, giving Mutex" << endl;
                xSemaphoreGive(twiMutex); // Use of twi is complete, free mutex
                Serial << "Finished a full round and Gave Mutex" << endl;
                
                // Grounded pin and current pin are not used in analysis so the 0th measurement
                // should be grounded pin +2 or current pin + 1.
                for (uint8_t i=0;i<14;i++) 
                {
                    skipCycleVals[i] = cycleVals[(currPinIndex + 1 + i) % 16];
                    #ifdef DEBUG_READMATERIAL
                    Serial << skipCycleVals[i] << endl;
                    #endif
                };
                Serial << "end skip" << endl;

                // Finds all the deltaV values used and stores them in a local data storage array,
                // ordered according to which of the 16 energization states is being recorded.
                for (uint8_t i=0;i<13;i++) 
                {
                    measure[cycleIndex*13+i] = skipCycleVals[i+1] - skipCycleVals[i];
                };

                cycleIndex++; // Advances to the next energization state.
                if (cycleIndex == 16)
                {
                    Serial << "Finished a full measurement" << endl;
                    cycleIndex = 0; // Resets energization marker to ground pin n=0
                    state = 3; // Change to publish value state
                }
            }
        }

        // Pushes all locally stored measurements to a global array accessable by the webpage task
        else if (state == 3) 
        {
            #ifdef DEBUG_READMATERIAL
            Serial << "publishing values" << endl;
            Serial << dataAvailable.get() << endl;
            #endif
            if (dataAvailable.get()) // Checks if the webpage task is currently attempting to read values
            {
                dataAvailable.put(false); // Lower the flag to avoid corrupted data
                #ifdef DEBUG_READMATERIAL
                Serial << "Took dataMutex" << endl;
                #endif
                // Record every datapoint
                for (uint8_t n=0;n<208;n++) 
                {
                    publish[n] = measure[n];
                    #ifdef DEBUG_READMATERIAL
                    Serial << measure[n] << endl;
                    #endif
                }
                dataAvailable.put(true); // data is no longer being used, raise the allow flag
                #ifdef DEBUG_READMATERIAL
                Serial << "Gave dataMutex" << endl;
                #endif
                state = 1; // go back to energizing the newest state
            }
            else {
                Serial << "Failed to take dataMutex" << endl;
            }
        }
        vTaskDelay(50/portTICK_PERIOD_MS); // Delay 50ms
    }
}

/*!
* @brief Task to handle controlling the table position using an IMU and two motors
* @details Has PID control on both motors to attempt to reach the desired setpoint and measured by a BNO055A IMU.
* @param p_params void*, unused.
*/
void task_controlMotors(void *parameter) {
    float x_angle, y_angle, xTargetAngle, yTargetAngle, xAngleErr, yAngleErr, errSumX,errSumY, effX, effY;
    float xAngleErrOld,yAngleErrOld;
    int16_t encoderXTicks, encoderYTicks, errX, errY;
    uint8_t pwm_X, pwm_Y;
    const float KP = 40; // Proportional gain for speed control
    const float KI = 0.1; // Integral gain for speed control
    const float KD = 10; // Integral gain for speed control
    
    uint8_t maxAngle = 10; // software limit for desired angle
    xTargetAngle = 0.0; // Target angle is 0 degrees by default
    yTargetAngle = 0.0; // Target angle is 0 degrees by default
    
    MOTOR_brake(motorXPin1, motorXPin2, 0, 1); // Initially stop both motors
    MOTOR_brake(motorYPin1, motorYPin2, 2, 3);

    uint8_t state = 0;
    for (;;) {
        // Initial State waits for IMU to initialize
        if (state == 0) {
            if (xSemaphoreTake(twiMutex,5) == pdTRUE) // Takes the mutex and returns true if successful
            {
                // Only advance states if IMU sucessfully initializes
                if (!IMU_init(5)) {
                    Serial.println("Failed to initialize IMU!");
                }
                else {
                    Serial.println("IMU initialized.");
                    state = 1;
                }
                xSemaphoreGive(twiMutex);
            }
        }
        // After Initialization, control the motor according to setpoint
        else if (state == 1) {
            /* get the currentl angles of the platform*/
            if (xSemaphoreTake(twiMutex,5) == pdTRUE) 
            {
                IMU_getAngles(x_angle, y_angle);
                xSemaphoreGive(twiMutex);
            }
            // if (abs(x_angle) > 15.0f || abs(y_angle) > 15.0f) {
            //     // If tilt angle exceeds 15 degrees, stop motors for safety
            //     MOTOR_brake(motorXPin1, motorXPin2, 0, 1);
            //     MOTOR_brake(motorYPin1, motorYPin2, 2, 3);
            //     Serial.println("Tilt angle too high! Motors stopped for safety.");
            //     vTaskDelay(500 / portTICK_PERIOD_MS);  // Delay for 500 ms
            //     continue; // Skip the rest of the loop
            // }

            // Adopts targets communicated by the webpage task
            xTargetAngle = xBar.get()*maxAngle; // Centroid values communicated are from -1 to 1
            yTargetAngle = yBar.get()*maxAngle;

            // Determine error
            xAngleErr = xTargetAngle - x_angle;
            yAngleErr = yTargetAngle - y_angle;
            
            // Constrain Integral Error to produce at most max effort to eliminate run-away integral control
            errSumX = constrain(errSumX + xAngleErr,-2550,2550);
            errSumY = constrain(errSumY + yAngleErr,-2550,2550);
            #ifdef DEBUG
            Serial << endl;
            Serial << xAngleErr << " " << yAngleErr << endl;
            Serial << errSumX << " " << errSumY << endl;
            #endif
            // Calculate appropriate effort
            effX = xAngleErr*KP + errSumX*KI + (xAngleErr-xAngleErrOld)*KD;
            effY = yAngleErr*KP + errSumY*KI + (yAngleErr-yAngleErrOld)*KD;
            
            // Assign previous error for use in derivative control
            xAngleErrOld = xAngleErr;
            yAngleErrOld = yAngleErr;

            // constrain effort to maximum allowed
            pwm_X = constrain((abs(effX)), 0, 255);
            pwm_Y = constrain((abs(effY)), 0, 255);
            
            #ifdef DEBUG
            Serial.print(pwm_X);
            Serial.print(" ");
            Serial.println(pwm_Y);
            #endif

            /* control motor X*/
            if (effX > 0) {
                MOTOR_forward(motorXPin1, motorXPin2, 0, 1, pwm_X);
            } else if (effX < 0) {
                MOTOR_reverse(motorXPin1, motorXPin2, 0, 1, pwm_X);
            } else {
                MOTOR_brake(motorXPin1, motorXPin2, 0, 1); // Shouldn't get here
            }
            /* control motor Y*/
            if (effY > 0) {
                MOTOR_reverse(motorYPin1, motorYPin2, 2, 3, pwm_Y);
            } else if (effY < 0) {
                MOTOR_forward(motorYPin1, motorYPin2, 2, 3, pwm_Y);
            } else {
                MOTOR_brake(motorYPin1, motorYPin2, 2, 3); // Shouldn't get here
            }
        }
        vTaskDelay(5/portTICK_PERIOD_MS); // Delay for 5 ms
    }
}

/*!
* @brief Task to handle the webpage for user interfacing.
* @details publishes all 208 datapoints used for 1 measurement and some flags used by an external python program
* to know whether to initialize a base value or make appropriate calculations. Additionally, uses args on the webpage
* to allow the user or external python program to set a desired table position.
* @param p_params void*, unused.
*/
void task_webserver (void* p_params)
{
    #ifdef DEBUG_WEB
    Serial << "handling Webpage" << endl;
    #endif
    // The server has been created statically when the program was started and
    // is accessed as a global object because not only this function but also
    // the page handling functions referenced below need access to the server
    server.on ("/", handle_DocumentRoot);
    server.on ("/data", handle_data);
    server.on ("/set", handleSetValues);
    server.on ("/flags", handleFlags);
    server.onNotFound (handle_NotFound);

    // Get the web server running
    server.begin ();
    Serial.println ("HTTP server started");

    // Task only has a single state, but changes behavior based on web requests as show above.
    for (;;)
    {
        // The web server must be periodically run to watch for page requests
        server.handleClient ();
        vTaskDelay (100/portTICK_PERIOD_MS); //delay 100 ms
    }
}

#ifndef SCANTWI
void setup() {
    Serial.begin(115200);
    delay(1000);

    twiMutex = xSemaphoreCreateMutex();

    /* Initialize motors  */
    MOTOR_init(motorXPin1, motorXPin2, 0, 1);
    MOTOR_init(motorYPin1, motorYPin2, 2, 3);

    /* initialize encoders*/
    ENCODER_init(encoderX, 27, 14);
    ENCODER_init(encoderY, 12, 13);

    pinMode(nSleepPin, OUTPUT);
    digitalWrite(nSleepPin, HIGH); // Wake up motor driver

    // Assign default share values
    initializeVFLG.put(false);
    readVFLG.put(false);
    xBar.put(0.0);
    yBar.put(0.0);
    dataAvailable.put(true);

    // Call function which gets the WiFi working
    setup_wifi();

    Serial.println("Setup complete.");

    /* Create control task */
    xTaskCreate(
         task_controlMotors,   // Task function
         "Control Motors",     // Name of the task
         4096,                 // Stack size (in words)
         NULL,                 // Task input parameter
         1,                    // Priority of the task
         NULL                 // Task handle
     );
    
    // Task which produces the blinking LED
    xTaskCreate (task_ReadMaterial, "EIT Read", 65536, NULL, 7, NULL);

    // Task which runs the web server.
    xTaskCreate (task_webserver, "Web Server", 8192, NULL, 3, NULL);
}

void loop() {
    vTaskDelay(portMAX_DELAY); //Don't need to run the default loop.
}

#else 
uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(PCA9956_ADDRESS);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {  // repeated start
    return 0xFF; // error marker
  }

  if (Wire.requestFrom(PCA9956_ADDRESS, (uint8_t)1) != 1) {
    return 0xFF; // error
  }
  return Wire.read();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // or your SDA/SCL

  delay(100);

  // Step 1: ping
  Wire.beginTransmission(PCA9956_ADDRESS);
  uint8_t err = Wire.endTransmission();
  
PCA9956 CurrCtrl (&Wire);

const uint8_t maxCurrent = 0xFF;
CurrCtrl.init(PCA9956_ADDRESS,maxCurrent,false);

if (err != 0) {
Serial.print("No ACK at 0x01, error = ");
Serial.println(err);
return;
}

Serial.println("ACK from 0x01, attempting to read MODE1...");
CurrCtrl.onLED(0);

  // Step 2: read MODE1 (reg 0x00)
}

void loop() {
  vTaskDelay(30000);
//   uint8_t count = 0;

//   Serial.println("Scanning...");

//   for (uint8_t address = 1; address < 127; address++) {
//     Wire.beginTransmission(address);
//     uint8_t error = Wire.endTransmission();

//     if (error == 0) {
//       Serial.print("TWI device found at 0x");
//       if (address < 16) Serial.print("0");
//       Serial.println(address, HEX);
//       count++;
//     }
//   }

//   if (count == 0) {
//     Serial.println("No TWI devices found");
//   }

//   Serial.println("Done.\n");
//   delay(2000);
}
#endif