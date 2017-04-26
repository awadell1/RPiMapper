//Handles Setting up the I/O and Interrupts for the Sonar Array and QTI Sensors
//The bulk of the code for handling the Sonar and QTI Sensors is located in Sensors.h
//All Macro's are defined in Sensors.h

//Libraries for Sensors
#include <avr/io.h>
#include <avr/interrupt.h>

//Set Up I/O and enable interrupts for running the Sonar Array 
void SetupSonar(void){
	// Setup Forward Sonar
	//Set Timer 1 Pre-scaler to 64
	TCCR1B |= (1  << CS11) | (1 << CS10);
	
	//Enables Pin Change interrupts for Port C
	PCICR |= (1 << PCIE1);

	//Set OCR1A to 30 mS
	OCR1A = 7500;
	
	//Set Timer 1 to Clear Timer on Compare 
	TCCR1B |= (1 << WGM12);
	
	//Turn on Interrupts for OCR1A, OCR1B
	TIMSK1 |= (1 << OCIE1A);
}

//Enable Pin change interrupts for the QTI Sensors
void SetupQTI(void){
	//Enable Pin Change Interrupt for QTI Port
	PCICR |= (1 << PCIE0);
	
	//Enable Interrupt on QTI Pins
	PCMSK0 |= ((1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2))  ;
}