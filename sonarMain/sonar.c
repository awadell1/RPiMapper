//Define CPU Speed
#define F_CPU 16000000UL

//Include relevant Libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Comment out to disable serial debugging
//#define debugOn

//Only include the serial library if debug mode is activated
#ifdef debugOn
#include "serial.h"
#endif

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

//Functions to take care of setup of Sonar Sensors
void SetupSonar(void);
void SetupQTI(void);

int main(void){
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

#endif /* INCFILE1_H_ */