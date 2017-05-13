#include "motor.h"

void setMotorSpeed(int left, int Right){
	// Update Motor Speeds
	// 100 -> Full Speed Forward
	// -100 -> Full Speed Reverse

	OCR0B = PWM_15 + LEFT_DIR * left * PWM_DS;
	OCR0A = PWM_15 + LEFT_DIR * left * PWM_DS;
}

void setupMotor(void){
	// Set Up Timer

	// Set Waveform Generation Mode to Normal
	TCCR0A &= ~((1<<WGM02) | (1<<WGM01) | (1<<WGM00));

	// Clear timer
	TCNT0 = 0;

	// Enable Overflow interrupt
	TIMSK0 |= (1<<TOIE0);
}