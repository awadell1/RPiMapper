//Include relevant Libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <Wire.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//Comment out to disable serial debugging
#define debugOn
#ifdef debugOn
	#define DEBUG_ODOMETRY // Enable Odometry Debugging
	//#define DEBUG_MOTOR
	//#define DEBUG_MOTOR_TIMING
	#define DEBUG_COMMS
#endif

// Define Buffer Sizes
#define BUFFER_LENGTH 64

// Sonar ISR
#include "sonarInterupt.h"
void SetupSonar(void);

// Motor
#include "motor.h"
#include "motorInterrupt.h"
extern volatile unsigned int leftSpeed;
extern volatile unsigned int rightSpeed;

// Define Motor States
// timerState 0 -> Generate Control Pulse
// timerState 1 -> Hold off for 20ms
#include "motorInterrupt.h"

// Odometry Setup
void SetupOdometry(void);
volatile unsigned long rightCount = 0;
ISR(INT1_vect){rightCount++;}

volatile unsigned long leftCount = 0;
ISR(INT0_vect){leftCount++;}

// I2C Response Buffer
char i2c_buff[32];
int i2c_buffer_ready = 0;
int i2c_buffer_len = 0;

// Function declarations
void SetupSonar(void);
void SetupI2C(void);
void SetupOdometry(void);
void str2int(int* num, const char str[], const int nNum);

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
	
	#ifdef debugOn
		Serial.print("Running Setup Functions\n");
	#endif
	SetupSonar();
	SetupI2C();
	SetupOdometry();
	setupMotor();
	setMotorSpeed(0,0);

	#ifdef debugOn
		Serial.print("Setup Functions Complete\n");
	#endif

	//////////////////////////////////////////////////////////////////////////
	
	//Enable Interrupts
	sei();

	//Begin Infinite Loop
	while(1)
    {
    	// Set LED On
		PORTB ^= (1<<PB5);

		char buff[32];
		sprintf(buff, "Left: %u Right: %u\n", leftCount, rightCount);
		Serial.print(buff);

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
	EICRA |= ((1<<ISC10) | (1<<ISC00));
	EIMSK |= ((1<<INT0) | (1<<INT1));

	// Set Pins to Input
	DDRD &= ~((1<<PD2) | (1<<PD3));
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
		sprintf(i2c_buff, "%u\n", SonarReading[sonar]);

	} else if(buff[0] == 'W'){
		// Set Wheel Speeds

		// Read in wheel speeds
		int speed[2] = {0};
		str2int(speed, buff+1, 2);

		// Update Wheel Speeds
		setMotorSpeed(speed[0], speed[1]);

		sprintf(i2c_buff, "Success\n");

	} else if(buff[0] == 'O'){
		// Write back odometry counts
		sprintf(i2c_buff, "%u %u\n", leftCount, rightCount);

		#ifdef DEBUG_ODOMETRY
			char buff[32];
			sprintf(buff, "Left: %u Right: %u\n", leftCount, rightCount);
			Serial.print(buff);
		#endif

	} else if(buff[0] == 'S'){
		// Write back message
		sprintf(i2c_buff, "Hello World\n");

	} else {
		// Write back error
		sprintf(i2c_buff, "ERROR\n");
	}
	
	#ifdef DEBUG_COMMS
		// Report Motor Speeds
		char debug_buff[64];
		sprintf(debug_buff, "CMD: [%s]\n", buff);
		Serial.print(debug_buff);

		sprintf(debug_buff, "RESP: [%s]\n", i2c_buff);
		Serial.print(debug_buff);
	#endif

	i2c_buffer_ready = 0; // Mark Arduino as ready
	return;
}

void setMotorSpeed(int left, int right){
	// Update Motor Speeds
	// 100 -> Full Speed Forward
	// -100 -> Full Speed Reverse

	leftSpeed = PWM_15 + LEFT_DIR * left * PWM_DS;
	rightSpeed = PWM_15 + RIGHT_DIR * right * PWM_DS;

	#ifdef DEBUG_MOTOR
		// Report Motor Speeds
		char buff[64];
		sprintf(buff, "Left Motor Speed: %d Timer %d\nRight Motor Speed: %d Timer %d\n",
			left, leftSpeed, right, rightSpeed);
		Serial.print(buff);
	#endif
}

void setupMotor(void){
	// Set Up Timer

	// Set Waveform Generation Mode to Normal
	TCCR0A &= ~((1<<WGM02) | (1<<WGM01) | (1<<WGM00));

	// Clear timer
	TCNT0 = 0;

	//Set Motor Pins as Outputs
	DDRD |= (1<<PD5) | (1<<PD6);

	// Enable Overflow interrupt
	TIMSK0 |= (1<<TOIE0) | (1<<OCF0A) | (1<<OCF0B);

	#ifdef DEBUG_MOTOR
		Serial.print("Motor Setup\n");
	#endif

	// Enable Timer
	TCCR0B |= (1<<CS00);
}
void str2int(int* num, const char str[], const int nNum){
// str2int: Extracts at most n int number from string

	// Pointer to start of unparsed str
	char* pEnd;
	
	// Get first number
	num[0] = strtol(str, &pEnd, 10);
	
	// Get the rest of the numbers
	for(int i = 1; i < nNum; i++){
		num[i] = strtol(pEnd, &pEnd, 10);
	}
}