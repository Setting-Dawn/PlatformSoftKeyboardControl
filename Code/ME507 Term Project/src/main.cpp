
#include <Arduino.h>
#include "PrintStream.h"
#include <WiFi.h>
#include <WebServer.h>
#include "ADC128D818.h"
#include "CD74HC4067SM.h"
#include "PCA9956.h"
#include "EITwebhost.h"
#include "taskshare.h"
#include "taskqueue.h"
#include "shares.h" 

// LED pin
const uint8_t LED_PIN = 2;

// Multiplexer control pins
const uint8_t s0_PIN = 26;
const uint8_t s1_PIN = 25;
const uint8_t s2_PIN = 33;
const uint8_t s3_PIN = 32;
const uint8_t MultiEnable_PIN = 35;

// ADC I2C Addresses
const uint8_t ADC_1ADDRESS = 0x1D;
const uint8_t ADC_2ADDRESS = 0x1F;

// PCA9956BTWY Addresses
const uint8_t PCA9956_ADDRESS = 0x01;

// Motor control pins
const uint8_t FAULT_PIN = 4;
const uint8_t NSLEEP_PIN = 2;
const uint8_t MOTOR_X_1 = 31;
const uint8_t MOTOR_X_2 = 30;
const uint8_t MOTOR_Y_1 = 28;
const uint8_t MOTOR_Y_2 = 27;


// A share which holds whether the external program needs to initialize
extern Share<bool> initializeVFLG;
// A share which holds whether the external program should read voltages
extern Share<bool> readVFLG;
// A share which holds whether the external program has communicated how the connections are defined
extern Share<bool> reMapCompleteFLG;
// A share which holds how the adc1 connections are defined
extern Share<uint8_t> adc1PinMap[8];
// A share which holds how the adc2 connections are defined
extern Share<uint8_t> adc2PinMap[8];
// A share which holds how the current connections are defined
extern Share<uint8_t> currPinMap[16];
// A share which holds whether the external program has communicated how the connections are defined
extern Share<double> publishDeltaV[208];

/*! @brief function to cycle the built-in LED in a set patern to aid debugging
* @details Blinks a pre-set desired morse code message to crashes externally diagnosable.
*
* @param p_params unused void pointer
*/ 
void heartbeat(void* p_params) {
    digitalWrite (LED_PIN, LOW);

    // define base dot length as 225ms
    const uint8_t sPulse = 225;

    // dots and dashes for "42" in morse
    const uint8_t pips[11] = {1,1,1,1,3,0,1,1,3,1,1};
    uint8_t n = 0;

    for (;;) {
        // Check if the message has been run, if so: delay before looping
        if (n == 11) {
            n = 0;
            digitalWrite (LED_PIN, LOW);
            vTaskDelay(sPulse*6);
            continue;
        }
        // If there should not be a dot or dash, turn off for a short pulse
        else if (pips[n]==0) {
            digitalWrite (LED_PIN, LOW);
            vTaskDelay(sPulse);
        }
        // Otherwise, turn the LED on for the appropriate length of time
        else {
            digitalWrite (LED_PIN, HIGH);
            vTaskDelay(sPulse*pips[n]);
        }
        
        // Write the LED to LOW for a short pulse between each increment
        digitalWrite (LED_PIN, LOW);
        vTaskDelay(sPulse);
        n++;
    }
}

void task_ReadMaterial(void* p_params) {
    ADC128D818 ADC_1 (ADC_1ADDRESS);
    ADC_1.begin();
    ADC128D818 ADC_2 (ADC_2ADDRESS);
    ADC_2.begin();
    CD74HC4067SM Multiplex (s0_PIN,s1_PIN,s2_PIN,s3_PIN,MultiEnable_PIN);
    PCA9956 CurrCtrl (&Wire);
    CurrCtrl.init(PCA9956_ADDRESS,0xFF); // Initialize current control address and max brightnes
    uint8_t currPinIndex;
    uint8_t GND_PinIndex;
    uint8_t adc1Mapping[8];
    uint8_t adc2Mapping[8];
    uint8_t currMapping[16];
    double cycleMeas[16] = {0};
    double measure[208] = {0};

    uint8_t state = 0;
    for (;;) {
        switch (state){
            case 0: // Idle until Signal from External Computer
                if (reMapCompleteFLG.get()) {
                    adc1PinMap.get(adc1Mapping);
                    adc2PinMap.get(adc2Mapping);
                    uint8_t currTemp[16] = currPinMap.get();
                    for (uint8_t n = 0;n<16;n++) {
                        currMapping[n] = currTemp[n];
                    }
                    state = 1;
                }
            case 1:
                for (uint8_t n = 0;n <16;n++) {
                    currPinIndex = (n + 1) % 16;
                    GND_PinIndex = n;
                    for (uint8_t i = 0;i<8;i++) {
                        cycleMeas[adc1Mapping[i]] = ADC_1.readConverted(i);
                        cycleMeas[adc2Mapping[i]] = ADC_2.readConverted(i);
                    }
                    measure[n*13] = *cycleMeas;

        }

        }
    }
}

/** @brief   Task which sets up and runs a web server.
 *  @details After setup, function @c handleClient() must be run periodically
 *           to check for page requests from web clients. One could run this
 *           task as the lowest priority task with a short or no delay, as there
 *           generally isn't much rush in replying to web queries.
 *  @param   p_params Pointer to unused parameters
 */
void task_webserver (void* p_params)
{
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

    for (;;)
    {
        // The web server must be periodically run to watch for page requests
        server.handleClient ();
        vTaskDelay (100);
    }
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay (100);
    while (!Serial) { }                   // Wait for serial port to be working
    delay(1000);
    Serial << "Would you like to play a game? [y/n]" << endl;

    // Call function which gets the WiFi working
    setup_wifi();
    
    // Set up the pin for the blue LED on the ESP32 board
    pinMode (LED_PIN, OUTPUT);
    digitalWrite (LED_PIN, LOW);

    // Task which produces the blinking LED
    xTaskCreate (heartbeat, "Pulse", 1024, NULL, 2, NULL);

    // Task which runs the web server.
    xTaskCreate (task_webserver, "Web Server", 8192, NULL, 11, NULL);
}

void loop() {
    // put your main code here, to run repeatedly:
    vTaskDelay(300000);
}
