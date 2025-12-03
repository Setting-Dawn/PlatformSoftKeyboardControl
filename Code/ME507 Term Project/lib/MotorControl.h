/**
 * @file MotorControl.h
 * @author Jake Rowen
 * @brief Header file for motor PID controller
 * Calculates control output based on target and current values.
 * @version 1.0
 * @date 2024-06-15
 */

#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H
#include <Arduino.h>
/* MOTOR CONTROL CLASS */

class MotorControl {
    private:
        float kp; // Proportional gain
        float ki; // Integral gain
        float kd; // Derivative gain
        float Error; // Error value
        float Esum; // Integral of error
    public:
        MotorControl(float p, float i, float d);
        float compute(float target, float current, float deltaTime);
};

