#ifndef GPIO_H
#define GPIO_H

#include <time.h>
/*! \file Gpio.h
 */

/*! GpioVal enum for gpio value. */
enum GpioVal
{ 
	/*! gpio value is high. */
	HIGH, 
	/*! gpio value is low. */
	LOW,	
	/*! gpio value is in unknown state?! */
	UNKNOWNVAL	
};

/*! GpioEdge enum for gpio input trigger edge. */
enum GpioEdge
{
	/*! gpio input trigger on rising edge. */
	RISING,	
	/*! gpio input trigger on falling edge. */
	FALLING,
	/*! gpio input trigger on both edge. */
	BOTH		
};

class Gpio
{
public:
	Gpio(unsigned int pinNum, bool isOut = true);
	~Gpio();
	
	int GetValue(GpioVal & value);
	
	int SetEdge(GpioEdge edge);
	int SetValue(GpioVal value);
	int SetDir(bool out); 
	
	int Poll(int usec, struct timespec & timeOfInterrupt);
	
private:
	bool _isOut;
	int _fd;
	GpioVal _curVal;
	unsigned int _pinNum;
	
	int Export();
	int Unexport();
	void OpenFd();
	void CloseFd();
	
};

#endif
