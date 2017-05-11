#ifndef ODOMETRY_H
#define ODOMETRY_H
//Include General Libraries for interfacing with Arduino
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define HOLD_OFF 2

// Define Odometry Pins
// Left Wheel	INT0	PD2		D2
// Right Wheel	INT1	PD3		D3
volatile unsigned long leftCount = 0;
volatile unsigned long rightCount = 0;

ISR(INT0_vect){
	leftCount++;

	char buff[32];
	sprintf(buff, "Left Count: %u\n", leftCount);
	Serial.print(buff);

	_delay_us(HOLD_OFF);
}

ISR(INT1_vect){
	rightCount++;

	char buff[32];
	sprintf(buff, "Right Count: %u\n", rightCount);
	Serial.print(buff);

	_delay_us(HOLD_OFF);
}
#endif