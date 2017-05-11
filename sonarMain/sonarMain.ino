//Include relevant Libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <Wire.h>

//Comment out to disable serial debugging
#define debugOn

// Define Buffer Sizes
#define BUFFER_LENGTH 64

// Odometry Setup
#include "odometryInterupt.h"
void SetupOdometry(void);
extern volatile unsigned long leftCount;
extern volatile unsigned long rightCount;

// Sonar ISR
#include "sonarInterupt.h"
void SetupSonar(void);


// I2C Response Buffer
char i2c_buff[32] = {0};
int i2c_buffer_ready = 0;
int i2c_buffer_len = 0;

extern volatile unsigned long SonarReading[];

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
	SetupOdometry();

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
	
	//Enables Pin Change interrupts for Port B, C and D
	PCICR |= (1 << PCIE0) | (1 << PCIE1) |  (1 << PCIE2);

	//Set OCR1A to 30 mS
	OCR1A = 7500;

	//Set Timer 1 to Clear Timer on Compare 
	TCCR1B |= (1 << WGM12);
	
	//Turn on Interrupts for OCR1A, OCR1B
	TIMSK1 |= (1 << OCIE1A);
}

void SetupI2C(void){
	// Enable Wire and set address to 1
	Wire.begin(0x05);

	// Set Handler
	Wire.onRequest(I2C_Request);
	Wire.onReceive(I2C_Receive);
}

void SetupOdometry(void){
	// Enable Pin Change interrupts
	EICRA |= (1<<ISC10) | (1<<ISC00);
	EIMSK |= (1<<INT0) | (1<<INT1);
}

void I2C_Request(){
	if(i2c_buffer_ready == 0){
		// Write next char
		Wire.write(i2c_buff);
	} else {
		Wire.write("Busy\n");
	}
	return;
}

void I2C_Receive(int numBytes){
	char buff[32] = {0};

	// Read at most 32 bytes
	if(numBytes > 32) numBytes = 32;

	for(int i = 0; i < numBytes; i++){
		buff[i] = Wire.read();
	}

	// Parse Request
	i2c_buffer_ready = -1; // Mark Arduino as busy
	if(buff[0] == 's' && numBytes >= 2){
		// Read sonar id
		int sonar;
		sscanf(buff+1, "%d", &sonar);

		// Write back sonar reading
		memset(i2c_buff, 0, sizeof(i2c_buff));
		sprintf(i2c_buff, "%u\n", SonarReading[sonar]);
	} else if(buff[1] == 'O'){
		// Write back odometry counts
		memset(i2c_buff, 0, sizeof(i2c_buff));
		sprintf(i2c_buff, "%u %u\n", leftCount, rightCount);
	} else if(buff[1] == 'S'){
		// Write back message
		sprintf(i2c_buff, "Hello World\n");
	} else {
		// Write back error
		sprintf(i2c_buff, "ERROR\n", SonarReading[0]);
	}
	i2c_buffer_ready = 0; // Mark Arduino as ready
	return;
}