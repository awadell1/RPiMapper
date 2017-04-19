//Contains Functions for Handling State Changes

//Include for access to motor control functions
#include "Motors.h"

//Include for access to relevant macros
#include "States.h"

//Initialize variable to store the desired spin direction during Scan
volatile unsigned short LastSeen = SpotRight;


//Changes Robot State to Scan Mode
void Scan(void){
	//Spin in the direction that the target was last seen (Or away from the boundary)
	if(LastSeen == SpotRight){
		//Last seen on Right -> Spin Right
		setMotorSpeed(LeftMotor | DriveForward, MotorMaxSpeed);
		setMotorSpeed(RightMotor | DriveBackward, MotorMaxSpeed);
	}
	else
	{
		//Last Seen on Left -> Spin Left
		setMotorSpeed(LeftMotor | DriveBackward, MotorMaxSpeed);
		setMotorSpeed(RightMotor | DriveForward, MotorMaxSpeed);	
	}
}
	
//Changes Robot State to Attack Mode
void Attack(void){
	//Drive Forwards
	setMotorSpeed(LeftMotor | DriveForward, MotorMaxSpeed);
	setMotorSpeed(RightMotor | DriveForward, MotorMaxSpeed);
}

//Changes the Robot State to Homing Mode and Handles Course Corrections
void Home(const unsigned long SonarReading[]) {
	//Create Mask to store Sonar Reading validity -> 1: Valid 0: Invalid
	unsigned short SonarMask = (1 << 0) | (1 << 1) | (1 << 2);
	
	//Initialize uint to store the sum of valid sonar readings
	unsigned int TotalSonar = 0;

	//Find the sum of all Valid Sonar Reading and note non-valid readings in the SonarMask
	for (int SonarIndex = 0; SonarIndex < 3; SonarIndex++){
		//Check if Sonar Reading valid
		if(SonarReading[SonarIndex] != SonarError){
			//Valid Reading -> Add to Total
			TotalSonar += SonarReading[SonarIndex];
		}
		else{
			//Invalid Reading -> Note in Sonar Mask
			SonarMask &= ~(1 << SonarIndex);
		}
	}
	

	//Initialize variable to store Centroid
	float Centroid = 0;
	
	//Compute the Sonar Centroid
	if(!(SonarMask & ~(1 << SonarLeft)) && !(SonarMask & ~(1 << SonarRight))){
		//Both SonarLeft and SonarRight are Valid  -> Use full Equation
		Centroid = ((float)SonarReading[SonarLeft] - (float)SonarReading[SonarRight])/((float)TotalSonar);
	}
	else if(!(SonarMask & ~(1 << SonarLeft))){
		//Only Sonar Left is Valid -> Exclude Sonar Right Reading
		Centroid = ((float)SonarReading[SonarLeft])/((float)TotalSonar);
	}
	else if(!(SonarMask & ~(1 << SonarRight))){
		//Only Sonar Right is Valid -> Exclude Sonar Left Reading
		Centroid = (-(float)SonarReading[SonarRight])/((float)TotalSonar);
	}
	
	//Respond to Centroid Value
	if(Centroid <= -HardTurn){
		//Target is on the far Right -> Spin Right
		setMotorSpeed(LeftMotor | DriveForward,MotorMaxSpeed);
		setMotorSpeed(RightMotor |  DriveBackward, MotorMaxSpeed);
		LastSeen = SpotRight;
	}
	else if (Centroid >= HardTurn){
		//Target on far left -> Spin Left
		setMotorSpeed(LeftMotor | DriveBackward,MotorMaxSpeed);
		setMotorSpeed(RightMotor |  DriveForward, MotorMaxSpeed);

		LastSeen = SpotLeft;
	}
	else if (Centroid <= 0){
		//Target is to the left -> Vear Left
		setMotorSpeed(LeftMotor | DriveForward,MotorMaxSpeed);
		setMotorSpeed(RightMotor |  DriveForward, MotorMaxSpeed - (unsigned int)(Centroid * TurnRate));
	}
	else{
		//Target is to the right -> Vear Right
		setMotorSpeed(LeftMotor | DriveForward,MotorMaxSpeed - (unsigned int)(Centroid * TurnRate));
		setMotorSpeed(RightMotor |  DriveForward, MotorMaxSpeed);
	}
}