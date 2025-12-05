/**
 * @file MotorControl.cpp
 * @author Jake Rowen
 * @brief Implementation file for motor PID controller
 * Calculates control output based on target and current values.
 * @version 1.0
 * @date 2024-06-15
 */

#include "MotorControl.h"
#include <Arduino.h>
#include <IMU.h>


/* MOTOR CONTROL CLASS IMPLEMENTATION */

MotorControl::MotorControl(float p, float i, float d) 
    : kp(p), ki(i), kd(d), Error(0.0), Esum(0.0) {}
    float MotorControl::compute(float target, float current, float deltaTime) {
    float previousError = Error;
    Error = target - current;
    Esum += Error * deltaTime;
    float dError = (Error - previousError) / deltaTime;
    float output = (kp * Error) + (ki * Esum) + (kd * dError);
    return output;
}