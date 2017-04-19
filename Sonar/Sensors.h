//Handles polling of the Sonar Sensors and Response to the QTI Sensors
//Also contains macros defining behavior and pin locations


#ifndef SONAR_H
#define SONAR_H

//Include for access to Motor Speed Control Functiona and related macros
#include "Motors.h"

//Include for access to LastSeen and related Macros
#include "States.h"
//////////////////////////////////////////////////////////////////////////
////					SENSOR CONFIG DEFINITIONS					//////
//////////////////////////////////////////////////////////////////////////

//The Number of Sonar Sensors Used
#define nSonar 3

//Define Sonar Pins
#define Sonar1 (1 << PC0)
#define Sonar2 (1 << PC1)
#define Sonar3 (1 << PC2)

//Define the Error Value used to indicate invalid readings
#define SonarError 65355

//Define the Reading the Sonar can accurately make
#define MaxDistance 4370	//Per the Datasheet the sonar can only measure up to 3 m

//Define QTI Pins
#define QTILeft (1 << PB0)
#define QTIRight (1 << PB2)
#define QTIBack (1 << PB1)

//////////////////////////////////////////////////////////////////////////

//Include General Libraries for interfacing with Arduino
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Initialize volatile long array to store Sonar Readings
volatile unsigned long SonarReading[nSonar] = {0};
	
//Initialize volatile long array to store new Sonar Reading pre-verification
volatile unsigned long SonarReadingNew[nSonar] = {0};

//Initialize volatile long array to store start times for Sonar Readings
volatile unsigned long SonarReadingStart[nSonar] = {0};

//Initialize register to store prior states of PORT C
volatile unsigned short SonarPinsLast = 0;

//Interrupt to handle cleanup of sonar reading and initialize the next reading
ISR(TIMER1_COMPA_vect){
	//Disable interrupts for Sonar Pins
	PCMSK1 &= ~(Sonar1 | Sonar2 | Sonar3);
	
	//Clean up Sonar Readings
	for(int SonarIndex = 0; SonarIndex < nSonar; SonarIndex++){
		
		//If Sonar Measurement > MaxDistance Or Zero -> Bad Reading
		if((SonarReadingNew[SonarIndex] >= MaxDistance) || (SonarReadingNew[SonarIndex] == 0)){
			SonarReadingNew[SonarIndex] = SonarError;
		}
			
		//Transfer verified reading to SonarReading
		SonarReading[SonarIndex] = SonarReadingNew[SonarIndex];
	}
	
	//////////////////////////////////////////////////////////////////////////
	//Trigger the Next Sonar Readings
	//////////////////////////////////////////////////////////////////////////
	
	//Set Sonar Pins to Output
	DDRC |= Sonar1 | Sonar2 | Sonar3;
	
	//Set Sonar Pins low to force clean pulse
	PORTC &= ~(Sonar1 | Sonar2 | Sonar3);
	
	//Wait 2 uS
	_delay_us(2);

	//Set Sonar Pins High to start Pulse
	PORTC |= Sonar1 | Sonar2 | Sonar3;
	
	//Wait 5 uS per Sonar Datasheet
	_delay_us(5);
	
	//Set Sonar Pins Low to end pulse
	PORTC &= ~(Sonar1 | Sonar2 | Sonar3);
	
	//Set Sonar Pins to Input
	DDRC &= ~(Sonar1 | Sonar2 | Sonar3);
	
	//Set Timer 1 to Zero
	TCNT1 = 0;
	
	//Enable Pin Change Interrupts for Sonar Pins
	PCMSK1 |= Sonar1 | Sonar2 | Sonar3;
}

//ISR To Handle Detecting and Recording the Response Pulse from the Sonar Sensors
ISR(PCINT1_vect){
	//Read Port C
	uint8_t PinsCurrent = PINC;
	
	//////////////////////////////////////////////////////////////////////////
	//Detect Rising Edges
	//////////////////////////////////////////////////////////////////////////
	
	//Rising Edge if Pin is Currently High and used to be Low
	uint8_t PinRisingEdge = ~SonarPinsLast & PinsCurrent;
	
	//Check for Rising Edge on the Sonar1 Pin
	if(PinRisingEdge & Sonar1){
		//Start of Pulse on Sonar1 Received -> Record in SonarReadingStart Array
		SonarReadingStart[0] = TCNT1;
	}
	
	//Check for Rising Edge on the Sonar2 Pin
	if(PinRisingEdge & Sonar2){
		//Start of Pulse on Sonar2 Received -> Record in SonarReadingStart Array
		SonarReadingStart[1] = TCNT1;
	}
	
	//Check for Rising Edge on the Sonar3 Pin
	if(PinRisingEdge & Sonar3){
		//Start of Pulse on Sonar3 Received -> Record in SonarReadingStart Array
		SonarReadingStart[2] = TCNT1;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//Detect Falling Edges
	//////////////////////////////////////////////////////////////////////////
	
	//Failing Edge if Pin is Currently Low and Used to be High
	uint8_t PinFallingEde = ~PinsCurrent & SonarPinsLast;
	
	//Check for the Falling edge on the Sonar 1 Pin
	if(PinFallingEde & Sonar1){
		//End of Pulse on Sonar1 Received -> Compute Elapsed Time
		SonarReadingNew[0] = TCNT1 - SonarReadingStart[0];
	}
	
	//Check for the Falling edge on the Sonar 2 Pin
	if(PinFallingEde & Sonar2){
		//End of Pulse on Sonar 2 Received -> Compute Elapsed Time
		SonarReadingNew[1] = TCNT1 - SonarReadingStart[1];
	}
	
	//Check for the Falling edge on the Sonar 3 Pin
	if(PinFallingEde & Sonar3){
		//End of Pulse on Sonar 3 Received -> Compute Elapsed Time
		SonarReadingNew[2] = TCNT1 - SonarReadingStart[2];
	}	
	
	//////////////////////////////////////////////////////////////////////////
	
	//Update SonarPinsLast to Current Pin State
	SonarPinsLast = PinsCurrent;
	
}

//Interrupt to handle responding to the QTI Sensors
ISR(PCINT0_vect){
	//Disable the pin change interrupts for QTI pins
	PCMSK0 &= ~((1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2)) ;
	
	//Sample QTI Sensor Continuously and take evasive action until all sensors read black
	while(!(PINB & QTILeft) || !(PINB & QTIRight) || !(PINB & QTIBack)) {
		if (!(PINB & QTILeft) && (PINB & QTIRight)){
			//Line on Front Left Side -> Spin Right
			setMotorSpeed(RightMotor | DriveBackward, MotorMaxSpeed);
			setMotorSpeed(LeftMotor | DriveForward, MotorMaxSpeed);
			
			//Set Flag to Spin Right (away from boundary) during Scan
			LastSeen = SpotRight;
		}
		else if((PINB & QTILeft) && !(PINB & QTIRight)){
			//Line on Front Right Side -> Spin Left
			setMotorSpeed(LeftMotor | DriveBackward, MotorMaxSpeed);
			setMotorSpeed(RightMotor | DriveForward, MotorMaxSpeed);
			
			//Set Flag to Spin Left (away from boundary) during Scan
			LastSeen = SpotLeft;
		}
		else if(!(PINB & QTILeft) && !(PINB & QTIRight)){
			//Line dead ahead -> Drive Backwards
			setMotorSpeed(LeftMotor | DriveBackward, MotorMaxSpeed - 20); //Turn slightly left so that we don't end up bouncing
			setMotorSpeed(RightMotor | DriveBackward, MotorMaxSpeed);
		}
		else if (!(PINB & QTIBack)){
			//Line in Rear -> Drive Forwards
			setMotorSpeed(LeftMotor | DriveForward, MotorMaxSpeed);
			setMotorSpeed(RightMotor | DriveForward, MotorMaxSpeed);
		}	
	}
	
	//Re-enable pin change interrupts for QTI
	PCMSK0 |= ((1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2)) ;
}

//Functions to take care of setup of Sonar Sensors
void SetupSonar(void);
void SetupQTI(void);

#endif /* INCFILE1_H_ */