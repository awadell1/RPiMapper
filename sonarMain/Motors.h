/*
 * Motors.h
 *
 * Created: 11/22/2015 11:23:33 PM
 *  Author: Alex
 */ 


#ifndef MOTORS_H_
#define MOTORS_H_


//Macros for Motor Selection and Direction
#define LeftMotor		0b00000000
#define RightMotor		0b00000001
#define DriveForward	0b00100000
#define DriveBreak		0b01000000
#define DriveBackward	0b10000000

//Define Max Value for Motor Speed
#define MotorMaxSpeed 255

//////////////////////////////////////////////////////////////////////////
//Pin Definitions
//////////////////////////////////////////////////////////////////////////

//Motor Right:
//White Wire: Pin 11
//Blue Wire: Pin 3

//Motor Left
//White Wire: Pin 6
//Blue Wire: Pin 5

//////////////////////////////////////////////////////////////////////////




//Declare Functions for Motor Control

void SetupDriveMotors(void);

void setMotorSpeed(unsigned short MotorReg, unsigned short Speed);

#endif /* MOTORS_H_ */