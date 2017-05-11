//Include relevant Libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <Wire.h>
#include <SparkFunLSM9DS1.h>

//Comment out to disable serial debugging
#define debugOn

// Define Buffer Sizes
#define BUFFER_LENGTH 64

// Sonar ISR
#include "sonarInterupt.h"
void SetupSonar(void);

// IMU Setup
LSM9DS1 imu;
#define MAGNO_I2C	0x1E	// I2C Address of Magnometer
#define GYRO_I2C	0x6B	// I2C Address of Gyro/ Accelerometer
#define DECLINATION 12.0	// Difference between Magnetic North and True North in Ithaca NY 

// MATH Constants
#define DEG2RAD (PI/180)

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
	SetupIMU();
	
	//////////////////////////////////////////////////////////////////////////
	
	//Enable Interrupts
	sei();

	//Begin Infinite Loop
	while(1)
    {
    	Serial.print("Hello\n");

    	// Read IMU sensor Readings
    	//ReadIMU();

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
	PCICR |= (1 << PCIE0) | (1 << PCIE1) | (1 << PCIE0);

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

void SetupIMU(void){
	// Set IMU Mode to I2C
	imu.settings.device.commInterface = IMU_MODE_I2C;
  	
  	// Set IMU I2C Addresses
  	imu.settings.device.mAddress = MAGNO_I2C;
  	imu.settings.device.agAddress = GYRO_I2C;

  	// Start up IMU
  	if(~imu.begin()){
  		Serial.print("Failed to connect to IMU\n");
  		Serial.print("Check I2C connection\n");
  	}
}

void ReadIMU(void){
	// Check for new Gyro Data
	if(imu.gyroAvailable()) imu.readGyro();

	// Check for new Accelerometer Data
	if (imu.accelAvailable()) imu.readAccel();

	// Check for new Magnetometer Data
	if (imu.magAvailable()) imu.readMag();
}

void I2C_Request(){
	if(i2c_buffer_ready == 0){
		// Write back buffer
		Wire.write(i2c_buff);
		i2c_buffer_ready = -1;
	} else{
		// Report Busy Status
		Wire.write("BUSY\n");
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
		memset(i2c_buff, '\0', sizeof(i2c_buff));
		sprintf(i2c_buff, "%u\n", SonarReading[sonar]);
	} else if(buff[1] == 'H'){
		// Write back message
		sprintf(i2c_buff, "Hello World\n");
	} else if(buff[1] == 'G'){
		// Compute Gyro Readings in Radians
		float gyro[3] = {0};

		gyro[0] = DEG2RAD * imu.calcGyro(imu.gx);
		gyro[1] = DEG2RAD * imu.calcGyro(imu.gy);
		gyro[2] = DEG2RAD * imu.calcGyro(imu.gz);

		// Write back Gyro Readings
		memset(i2c_buff, '\0', sizeof(i2c_buff));
		sprintf(i2c_buff, "%3.2f %3.2f %3.2f\n", gyro[0], gyro[1], gyro[2]);
	} else if (buff[1] == 'M'){
		// Compute Magnetometer Readings
		float magno[3] = {0};

		magno[0] = imu.calcMag(imu.mx);
		magno[1] = imu.calcMag(imu.my);
		magno[2] = imu.calcMag(imu.mz);

		// Write back Magnetometer Readings
		memset(i2c_buff, '\0', sizeof(i2c_buff));
		sprintf(i2c_buff, "%3.2f %3.2f %3.2f\n", magno[0], magno[1], magno[2]);
	} else if(buff[1] == 'A') {
		// Compute Acceleration Data
		float accel[3] = {0};

		accel[0] = imu.calcAccel(imu.ax);
		accel[1] = imu.calcAccel(imu.ay);
		accel[2] = imu.calcAccel(imu.az);

		// Write back Magnetometer Readings
		memset(i2c_buff, '\0', sizeof(i2c_buff));
		sprintf(i2c_buff, "%3.2f %3.2f %3.2f\n", accel[0], accel[1], accel[2]);
	} else {
		// Write back error
		sprintf(i2c_buff, "ERROR\n", SonarReading[0]);
	}
	i2c_buffer_ready = 0; // Mark Arduino as ready
	i2c_buffer_len = strlen(i2c_buff);

	return;
}