//Include relevant Libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <Wire.h>

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
	SetupI2C();
	
	//////////////////////////////////////////////////////////////////////////
	
	//Enable Interrupts
	sei();

	//Begin Infinite Loop
	while(1)
    {
		//Required Delay for Sonar Loop, Do Not Remove or Reduce
		_delay_ms(30);
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

	//Set Sonar Pins to Input
	DDRC &= ~(Sonar1 | Sonar2 | Sonar3);
	
	//Turn on Interrupts for OCR1A, OCR1B
	TIMSK1 |= (1 << OCIE1A);
}

void SetupI2C(void){
	// Enable Wire and set address to 1
	Wire.begin(1);

	// Set Handler
	Wire.onReceive(I2C_Request);
}

void I2C_Request(int numBytes){
	// Report Recieving
	Serial.print("RCV: ");

	// Step through commands
	while(Wire.available()){
		// Read Next Commands
		int cmd = (int)Wire.read();
		Serial.print(cmd);
		Serial.print("\n");

		// Respond to Command
		if (cmd == 1){
			// Convert Sonar Readings to char array
			char sonar[1028]; memset(&sonar[0], 0, sizeof(sonar));
			for(int i = 0; i < nSonar; i++){
				snprintf(sonar+strlen(sonar), sizeof(sonar), "%u ", SonarReading[i]);
			}
			
			// Return Sonar Readings
			Wire.write(sonar, strlen(sonar));
			#ifdef debugOn
				Serial.print("\t Sonar:");
				Serial.print(sonar);
			#endif
		}
		else {
			// Report Error
			Wire.write("ERRORL Invalid Command");
		}

	}
}