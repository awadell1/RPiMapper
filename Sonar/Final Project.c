//Main Code Loop for Robot Control


//Comment out to disable serial debugging
//#define debugOn 


//Define CPU Speed
#define F_CPU 16000000UL

//Include relevant Libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "Motors.h"		//Functions to handle Motor Speed Control
#include "Sensors.h"	//Functions to handle Sonar Readings and QTI Response
#include "States.h"		//Function to define Robot States

#ifdef debugOn //Only include the serial library if debug mode is activated
#include "serial.h"
#endif

int main(void)
{
	//Start Serial Communication if Debugging Mode is activated
	#ifdef debugOn
		init_uart();
		printf("Debugging Mode is Active\r\n");
	#endif
	
	//Set Pin 13 (On-board LED) to output mode
	DDRB |= (1 << PB5);
	
	
	//////////////////////////////////////////////////////////////////////////
	/////						Run Set Up Functions					//////
	//////////////////////////////////////////////////////////////////////////
	
	SetupDriveMotors();
	SetupSonar();
	SetupQTI();
	
	//////////////////////////////////////////////////////////////////////////
	
	//Enable Interrupts
	sei();
	
	//Set start Mode to Scan
	Scan();
	
	//Define variable to store current robot state while debugging
	#ifdef debugOn
		//Variable to Store Current State during Debug
		unsigned short StateID = ScanState;
	#endif
	
	//Begin Infinite Loop
	while(1)
    {
		
		if ((SonarReading[SonarForward] == SonarError) &&  (SonarReading[SonarLeft] == SonarError) && (SonarReading[SonarRight] == SonarError)){
			//None of the Sonars see anything -> Switch to scan mode
			Scan();
			
			//Update Current State for Debugging
			#ifdef debugOn
				StateID = ScanState;
			#endif
		}
		else if ((SonarReading[SonarForward] >= FarDistance) &&  (SonarReading[SonarLeft] >= FarDistance) && (SonarReading[SonarRight] >= FarDistance)) {
			//None of the Sonars see anything -> Switch to scan mode
			Scan();
			
			//Update Current State for Debugging
			#ifdef debugOn
				StateID = ScanState;
			#endif

		}
		else if (SonarReading[SonarForward] < AttackDistance) {
			//Within Attack Range -> Trigger Attack State
			Attack();
			
			//Update Current State for Debugging
			#ifdef debugOn
				StateID = AttackState;
			#endif			

		}
		else{
			//Spotted Target but not within Attack Range -> Home in on target
			Home(SonarReading);
			
			//Update Current State for Debugging
			#ifdef debugOn
				StateID = HomeState;
			#endif
		}
		
		//Report Sonar Readings and Sate over Serial in CSV format -> Use ReadSonar.M to plot in realtime
		#ifdef debugOn
			printf("%u,",SonarReading[SonarForward]);
			printf("%u,",SonarReading[SonarLeft]);
			printf("%u,",SonarReading[SonarRight]);
			printf("%u\r\n",StateID);
			
			//Serial is really slow -> Need extra delay to transmit
			_delay_ms(100);
		#endif
		
		//Required Delay for Sonar Loop, Do Not Remove or Reduce
		_delay_ms(30);
		
		//Toggle LED to indicate end of cycle
		PORTB ^= (1<<PB5);
    }
}