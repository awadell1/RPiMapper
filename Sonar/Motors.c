//Handles setting Motor Speed via PWM

//Include General Libraries for I/O Access
#include <avr/io.h>
#include <avr/interrupt.h>

//Include Motors.h for access to Macros
#include "Motors.h"

//Set up PWM on the Control Pins for Each Motor
void SetupDriveMotors(void){
	//////////////////////////////////////////////////////////////////////////
	////						MOTOR LEFT								 /////
	//////////////////////////////////////////////////////////////////////////
	
	//Set both control pins as output
	DDRD |= (1 << PD5) | (1 << PD6);
	
	//Enable Timer 0 for PWM
	//Set Pre-Scaler to 1
	TCCR0B |= (1 << CS00);
	
	//Set Waveform generation Mode to Fast PWM
	TCCR0A |= (1 << WGM00) | (1 << WGM01);
	
	//Set Motor Speed to Zero
	OCR0A = 0;	OCR0B = 0;
		
	//Turn on PWM
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
	
	//////////////////////////////////////////////////////////////////////////
	////						MOTOR RIGHT								 /////
	//////////////////////////////////////////////////////////////////////////
	
	//Set both control pins as output
	DDRB |= (1 << PB3);
	DDRD |= (1 << PD3);
	
	//Enable Timer 2 for PWM
	//Set Pre=scaler to 1
	TCCR2B |= (1 << CS20);
	
	//Set Waveform Generation Mode to Fast PWM
	TCCR2A |= (1 << WGM20) || (1 << WGM21);
	
	//Set Motor Speed to Zero
	OCR2A = 0;	OCR2B = 0;
	
	//Turn on PWM
	TCCR2A |= (1 << COM2A1) | (1 << COM2B1);
}

void setMotorSpeed(unsigned short MotorReg, unsigned short Speed){
	//Select Motor (Left/Right) and Direction (Forward/Backwards/Brake) using MotorReg
	//Speed is bounded to 0 <-> 255 (Resolution of both 8-bit Timers)
	
	//Use PWM to modulate Motor Speed
	//Set Pin Low by Changing PWM OCRxx to zero -> Effectivly sets the Pin Low 
	//Also eliminates having to set and reset the WGM of the pins -> Shorter Execution time -> Faster Motor Response Time
	//Note: Low Side PWM is set first then High Side -> Reduces risk of shorting out the H-birdge
	
	switch(MotorReg) {
		//////////////////////////////////////////////////////////////////////////
		////							LEFT MOTOR							  ////
		//////////////////////////////////////////////////////////////////////////
		
		case (LeftMotor | DriveBackward) :	//Drive Motor Left - Backwards
			//Set White Control Line to Low
			OCR0A = 0;
			
			//Set Blue Control Line to High PWM
			OCR0B = Speed;
		break;
			
		case (LeftMotor | DriveBreak) :	//Drive Motor Left - Stop
			//Set both White and Blue Control Pins Low
			OCR0A = 0; OCR0B = 0;
		break;
			
		case (LeftMotor | DriveForward) :	//Drive Motor Left - Forwards
			//Set Blue Control Line to Low
			OCR0B = 0;
			
			//Set White Control Line to High PWM
			OCR0A = Speed;
		break;
		
		
		//////////////////////////////////////////////////////////////////////////
		////							Right MOTOR							  ////
		//////////////////////////////////////////////////////////////////////////
		
		case (RightMotor | DriveBackward) :	//Drive Motor Right - Backwards
			//Set White Control Line to Low
			OCR2A = 0;
			//Set Blue Control Line to High PWM
			OCR2B = Speed;
		break;
			
		case (RightMotor | DriveBreak) :	//Drive Motor Right - Stop
			//Set both White and Blue Control Pins Low
			OCR2A = 0; OCR2B = 0;
		break;
			
		case (RightMotor | DriveForward) :	//Drive Motor Right -  Forwards		
			//Set Blue Control Line to Low
			OCR2B = 0;
			
			//Set White Control Line to High PWM
			OCR2A = Speed;
		break;
	}
}