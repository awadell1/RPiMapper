#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "Sonar.h"

/*! \file Sonar.cpp */

/*! Polling timeout on each sonar reading. */
#define POLL_TIMEOUT 100

/*!
 * 	\class Sonar Sonar.h "Sonar.h"
 *	\brief This class is the class handle sonar related stuff.
 */

/*! \fn Sonar::Sonar(unsigned int gpioPinNum)
 * 	\brief A constructor for the Sonar class. It calls the constructor for Gpio.
 * 	\param gpioPinNum The gpio pin number that this sonar is plugged into.
 */
Sonar::Sonar(unsigned int gpioPinNum)
{
	_gpio = new Gpio(gpioPinNum);
	_minDist = 999;
	_maxDist = 0;
	isEnding = false;
}

/*! \fn Sonar::~Sonar()
 * 	\brief A destructor for the Sonar class. It calls the destructor for Gpio.
 */
Sonar::~Sonar()
{
	delete _gpio;
}

/*! \fn struct timespec Sonar::TimeDiff()
 * 	\brief A helper class to find out the time elapsed between the falling trigger and rising trigger.
 * 	\return the difference in time.
 */
struct timespec Sonar::TimeDiff()
{
	struct timespec temp;
	if ((_fallingTOI.tv_nsec - _risingTOI.tv_nsec) < 0)
	{
		temp.tv_sec = _fallingTOI.tv_sec - _risingTOI.tv_sec - 1;
		temp.tv_nsec = 1000000000 + _fallingTOI.tv_nsec - _risingTOI.tv_nsec;
	}
	else
	{
		temp.tv_sec = _fallingTOI.tv_sec - _risingTOI.tv_sec;
		temp.tv_nsec = _fallingTOI.tv_nsec - _risingTOI.tv_nsec;
	}
	return temp;
}

/*! \fn float Sonar::DisplayMeasurement()
 * 	\brief Calculate the distance measurement from the time of flight.
 * 	\return The distance calculated.
 */
float Sonar::DisplayMeasurement()
{
	if (TimeDiff().tv_sec == 0)
	{
		float time = TimeDiff().tv_nsec/1000000000.f;
		float dist = 340.29*(time/2);
		if (dist < 3.f && dist > 0)
		{
			if (dist < _minDist)
			{
				_minDist = dist;
			}
			if (dist > _maxDist)
			{
				_maxDist = dist;
			}
			#if 0
			// no longer want to print out the message.
			printf("Time taken is: %fs\n", time);
			printf("Dist is: %fm\t minDist: %fm\t maxDist: %fm\n", dist, _minDist, _maxDist);
			fflush(stdout);
			#endif
			return dist;
		}
	}
	return 0.f;
}

/*! \fn void Sonar::StartPulse()
 * 	\brief Start the pulse to request a sonar measurement.
 */
void Sonar::StartPulse()
{
	_gpio->SetDir(true);
	_gpio->SetValue(LOW);
	usleep(10);
	_gpio->SetValue(HIGH);
	usleep(5);
	_gpio->SetValue(LOW);
}

/*! \fn float Sonar::Run()
 * 	\brief The main loop/function of the Sonar class.
 * 	\return The distance measurement of from the sonar.
 */
float Sonar::Run()
{
	StartPulse();
	_gpio->SetEdge(RISING);
	usleep(500);
	if (_gpio->Poll(POLL_TIMEOUT, _risingTOI) > 0)
	{
		_gpio->SetEdge(FALLING);
		if (_gpio->Poll(POLL_TIMEOUT, _fallingTOI) <= 0)
		{
			return -1.f;
		}
	}
	else
	{
		return -1.f;
	}
	return DisplayMeasurement();
}
