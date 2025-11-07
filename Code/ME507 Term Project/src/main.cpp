
#include <Arduino.h>

// Motor control pins

const uint8_t FAULT_PIN = 4;
const uint8_t NSLEEP_PIN = 2;
const uint8_t MOTOR_X_1 = 31;
const uint8_t MOTOR_X_2 = 30;
const uint8_t MOTOR_Y_1 = 28;
const uint8_t MOTOR_Y_2 = 27;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hello World!");

}

void loop() {
  // put your main code here, to run repeatedly:
}
