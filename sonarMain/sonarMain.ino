//Include relevant Libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Comment out to disable serial debugging
#define debugOn

// Sonar ISR
#include "sonarInterupt.h"
void SetupSonar(void);

int main(void){
	//Start Serial Communication if Debugging Mode is activated
	#ifdef debugOn
		Serial.begin(9600);
		Serial.print("Debugging Mode is Active\n");
	#endif
	
	//Set Pin 13 (On-board LED) to output mode
	DDRB |= (1 << PB5);
	
	
	//////////////////////////////////////////////////////////////////////////
	/////						Run Set Up Functions					//////
	//////////////////////////////////////////////////////////////////////////
	
	SetupSonar();
	
	//////////////////////////////////////////////////////////////////////////
	
	//Enable Interrupts
	sei();

	//Begin Infinite Loop
	while(1)
    {
		//Report Sonar Readings and Sate over Serial in CSV format -> Use ReadSonar.M to plot in realtime
		#ifdef debugOn
			char buffer[126];
			sprintf(buffer, "%u, %u, %u\n", SonarReading[0], SonarReading[1],SonarReading[2]);
			Serial.print(buffer);
			
			//Serial is really slow -> Need extra delay to transmit
			_delay_ms(500);
		#endif
		
		//Required Delay for Sonar Loop, Do Not Remove or Reduce
		_delay_ms(30);
		
		//Toggle LED to indicate end of cycle
		PORTB ^= (1<<PB5);
    }
}

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