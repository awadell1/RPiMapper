#ifndef SONAR_H
#define SONAR_H

#include <stdio.h>
#include <time.h>
#include "Gpio.h"

/*! \file Sonar.h */

/*! The udp port number for sending sonar measurement. */
#define SONAR_PORT 8833
/*! time in usec before next measurement. */
#define SONAR_MEASURE_RATE 10000
/*! time in usec before switch to other sonar. */
#define SONAR_WAIT_TIME	10000

class Sonar
{
public:
	Sonar(unsigned int gpioPinNum);
	~Sonar();
	
	float Run();
	
	/*! Flag for getting ready to end this Sonar class. */
	bool isEnding;
	
private:
	Gpio * _gpio;
	float _minDist;
	float _maxDist;
	struct timespec _risingTOI;
	struct timespec _fallingTOI;
	
	struct timespec TimeDiff();
	float DisplayMeasurement();
	void StartPulse();
	
};

#endif
