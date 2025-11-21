
#include <Arduino.h>
#include "PrintStream.h"
#include "ADC128D818.h"
#include "CD74HC4067SM.h"
#include "PCA9956.h"

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

void task_ReadMaterial(void* p_params) {
    ADC128D818 ADC_1 (ADC_1ADDRESS);
    ADC128D818 ADC_2 (ADC_2ADDRESS);
    CD74HC4067SM Multiplex (s0_PIN,s1_PIN,s2_PIN,s3_PIN,MultiEnable_PIN);
    PCA9956 CurrCtrl (&Wire);
    CurrCtrl.init(PCA9956_ADDRESS,0xFF); // Initialize current control address and max brightnes
}

void task_comm(void* p_params) {
    const uint8_t valueCount = 208;
    uint8_t state = 0;
    for (;;)
    {
        switch (state)
        {
            case 1: //Send datasize
                Serial << "Sending count: " << valueCount << endl;
                state = 2;

            case 2: //Wait for confirm
                vTaskDelay(10); //
                state = 3;
            
            case 3: //Send data
                for (uint8_t n = 0; n <208;n++) {
                    Serial << data.get() << endl;
                }
        }
        Serial.readln
    }


}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(1000);
    Serial.println("Hello World!");

}

void loop() {
    // put your main code here, to run repeatedly:
    vTaskDelay(300000);
}
