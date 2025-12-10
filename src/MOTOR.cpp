/** Implementation of motor commands */

#include "MOTOR.h"
#include <Arduino.h>

/**
 * @brief Initialize motor control pins and PWM channels.
 * 
 * Configures the specified GPIO pins for motor output and sets up
 * two independent PWM channels at 25 kHz with 8-bit resolution.
 * 
 * @param pin1 GPIO pin for motor direction 1
 * @param pin2 GPIO pin for motor direction 2
 * @param channel1 PWM channel number for first direction (0-15)
 * @param channel2 PWM channel number for second direction (0-15)
 * 
 * @return void
 * 
 * @details Both PWM channels are configured with:
 *   - Frequency: 25 kHz
 *   - Resolution: 8-bit (0-255)
 *   - Duty cycle controlled via ledcWrite()
 */
void MOTOR_init(uint8_t pin1,uint8_t pin2, uint8_t channel1, uint8_t channel2) {
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    ledcSetup(channel1, 25000, 8);
    ledcSetup(channel2, 25000, 8);
    ledcAttachPin(pin1, channel1);
    ledcAttachPin(pin2, channel2);
}
    
/**
 * @brief Drive the motor forward at a specified speed.
 * 
 * Applies forward motion by writing the speed value to the forward channel
 * and setting the reverse channel to 0. The motor will move in the forward
 * direction proportional to the speed parameter.
 * 
 * @param pin1 GPIO pin for motor direction 1 (unused in this implementation)
 * @param pin2 GPIO pin for motor direction 2 (unused in this implementation)
 * @param channel1 PWM channel for forward motion
 * @param channel2 PWM channel for reverse motion
 * @param speed PWM duty cycle (0-255, where 255 is maximum speed)
 * 
 * @return void
 * 
 * @note To change direction, call MOTOR_reverse() or MOTOR_brake()
 * 
 * @see MOTOR_reverse(), MOTOR_brake()
 */
void MOTOR_forward(uint8_t pin1,uint8_t pin2, uint8_t channel1, uint8_t channel2, uint8_t speed) {
    ledcWrite(channel1, speed);
    ledcWrite(channel2, 0);
}

/**
 * @brief Drive the motor in reverse at a specified speed.
 * 
 * Applies reverse motion by writing the speed value to the reverse channel
 * and setting the forward channel to 0. The motor will move in the reverse
 * direction proportional to the speed parameter.
 * 
 * @param pin1 GPIO pin for motor direction 1 (unused in this implementation)
 * @param pin2 GPIO pin for motor direction 2 (unused in this implementation)
 * @param channel1 PWM channel for forward motion
 * @param channel2 PWM channel for reverse motion
 * @param speed PWM duty cycle (0-255, where 255 is maximum speed)
 * 
 * @return void
 * 
 * @note To change direction, call MOTOR_forward() or MOTOR_brake()
 * 
 * @see MOTOR_forward(), MOTOR_brake()
 */
void MOTOR_reverse(uint8_t pin1,uint8_t pin2, uint8_t channel1, uint8_t channel2, uint8_t speed) {
    ledcWrite(channel1, 0);
    ledcWrite(channel2, speed);
}

/**
 * @brief Apply electrical braking to stop the motor quickly.
 * 
 * Applies maximum PWM signal to both motor channels simultaneously, causing
 * them to work against each other and bring the motor to a rapid stop through
 * electrical braking. This is more effective than coasting to a stop.
 * 
 * @param pin1 GPIO pin for motor direction 1 (unused in this implementation)
 * @param pin2 GPIO pin for motor direction 2 (unused in this implementation)
 * @param channel1 PWM channel for forward motion
 * @param channel2 PWM channel for reverse motion
 * 
 * @return void
 * 
 * @details Both channels are set to 255 (maximum duty cycle), creating
 * opposing forces that provide rapid deceleration.
 * 
 * @see MOTOR_forward(), MOTOR_reverse()
 */
void MOTOR_brake(uint8_t pin1,uint8_t pin2, uint8_t channel1, uint8_t channel2) {
    ledcWrite(channel1, 255);
    ledcWrite(channel2, 255);
}