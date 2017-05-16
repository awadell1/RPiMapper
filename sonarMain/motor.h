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
#define PWM_15 93
#define PWM_DS 0.13

// Motor Direction 
#define LEFT_DIR 1
#define RIGHT_DIR -1

void setupMotor();
void setMotorSpeed(int left, int Right);

#endif