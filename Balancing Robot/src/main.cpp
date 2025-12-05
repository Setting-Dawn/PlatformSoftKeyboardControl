#include <Arduino.h>
#include "IMU.h"
#include "MOTOR.h"
#include "ENCODER.h"
#include <ESP32Encoder.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cmath>

/* main control loop for balancing robot */

uint8_t motorXPin1 = 19;
uint8_t motorXPin2 = 18;

uint8_t motorYPin1 = 17;
uint8_t motorYPin2 = 16;
ESP32Encoder encoderX;
ESP32Encoder encoderY;

void task_controlMotors(void *parameter) {
  float x_angle, y_angle, xTargetAngle, yTargetAngle, xAngleErr, yAngleErr;
  int16_t encoderXTicks, encoderYTicks, errX, errY;
  uint8_t pwm_X, pwm_Y;
  const uint8_t KP = 2; // Proportional gain for speed control

    while (true) {
      /* get the currentl angles of the platform*/
        IMU_getAngles(x_angle, y_angle);
        xTargetAngle = 0.0f; // Target angle is 0 degrees
        yTargetAngle = 0.0f; // Target angle is 0 degrees
        xAngleErr = xTargetAngle - x_angle;
        yAngleErr = yTargetAngle - y_angle;
      /* get the needed encoder ticks to zero*/
        encoderXTicks = round(xAngleErr*2.77778); // 1 degree = 2.77778 ticks (for 90 degree = 250 ticks)
        encoderYTicks = round(yAngleErr*2.77778);
        /* set PWM value (speed) for motors*/
        errX = encoderXTicks - ENCODER_getCount(encoderX);
        errY = encoderYTicks - ENCODER_getCount(encoderY);
        // Proportioal speed control
        pwm_X = constrain((abs(errX)*KP), 0, 255);
        pwm_Y = constrain((abs(errY)*KP), 0, 255);
        /* control motor X*/
        if (errX > 0) {
            MOTOR_forward(motorXPin1, motorXPin2, 0, 1, pwm_X);
        } else if (errX < 0) {
            MOTOR_reverse(motorXPin1, motorXPin2, 0, 1, pwm_X);
        } else {
            MOTOR_brake(motorXPin1, motorXPin2, 0, 1);
        }
        /* control motor Y*/
        if (errY > 0) {
            MOTOR_forward(motorYPin1, motorYPin2, 2, 3, pwm_Y);
        } else if (errY < 0) {
            MOTOR_reverse(motorYPin1, motorYPin2, 2, 3, pwm_Y);
        } else {
            MOTOR_brake(motorYPin1, motorYPin2, 2, 3);
        }

        vTaskDelay(5 / portTICK_PERIOD_MS);  // Delay for 5 ms
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (!IMU_init(30000)) {
        Serial.println("Failed to initialize IMU!");
        while (1);
    }

    Serial.println("IMU initialized.");

    /* Initialize motors  */
    MOTOR_init(motorXPin1, motorXPin2, 0, 1);
    MOTOR_init(motorYPin1, motorYPin2, 2, 3);

    /* initialize encoders*/
    ENCODER_init(encoderX, 27, 14);
    ENCODER_init(encoderY, 12, 13);

    Serial.println("Setup complete.");
}

void loop() {

}
