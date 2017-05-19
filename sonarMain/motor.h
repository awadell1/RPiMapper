#ifndef MOTOR_H
#define MOTOR_H

// Required Libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


// Define Motor Pins
// Left Motor	OCR0B	PD5		D5
// Right Motor	OCR0A	PD6		D6

// PWM Compare Levels
#define PWM_15 92.5		// Timer Value to generate a 1.5 ms Pulse
#define PWM_DS 0.125	// Amount to change Timer value per % motor speed

// Motor Direction: Flag to flip the "forward direction of the motor"
// 1: 1.7 ms Pulse -> Full Speed Forward
// -1: 1.3 ms Pulse -> Full Speed Forward
#define LEFT_DIR 1
#define RIGHT_DIR -1

// Function to set up timers required for motors
void setupMotor();

// Function to update motor speed
void setMotorSpeed(int left, int Right);

#endif