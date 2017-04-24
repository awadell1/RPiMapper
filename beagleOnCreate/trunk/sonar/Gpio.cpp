#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "Gpio.h"

/*! \file Gpio.cpp */

/*! The directory for sys_fs gpio handle. */
#define SYSFS_GPIO_DIR "/sys/class/gpio"
/*! The buffer size. */
#define MAX_BUF 64

/*!
 * 	\class Gpio Gpio.h "Gpio.h"
 *	\brief This class is the class to handle any gpio related actions.
 */

/*! \fn Gpio::Gpio(unsigned int pinNum, bool isOut)
 * 	\brief A constructor for Gpio class. Also export the port in the constructor.
 * 	\param pinNum the pin number for the gpio.
 *  \param isOut true if the gpio is initalized as output.
 */
Gpio::Gpio(unsigned int pinNum, bool isOut)
{
	_curVal = UNKNOWNVAL;		// Make it out of range to force write if out pin
	_fd = -1;				// Make it invalid to force initial open
	_pinNum = pinNum;

	Export();
	SetDir(isOut);	
}

/*! \fn Gpio::~Gpio()
 * 	\brief A destructor for Gpio class. Unexport here.
 */
Gpio::~Gpio()
{
	if( 0 <= _fd )
	{
		Unexport();
		CloseFd();
	}
}

/*! \fn int Gpio::Export()
 * 	\brief Export the gpio pin for sys_fs.
 * 	\return 0 on success, -1 on fail.
 */
int Gpio::Export()
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0)
	{
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", _pinNum);
	write(fd, buf, len);
	close(fd);
	return 0;
}

/*! \fn int Gpio::Unexport()
 * 	\brief Unexport the gpio pin for sys_fs.
 * 	\return 0 on success, -1 on fail.
 */
int Gpio::Unexport()
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0)
	{
		perror("gpio/unexport");
		return fd;
	}
	
	len = snprintf(buf, sizeof(buf), "%d", _pinNum);
	write(fd, buf, len);
	close(fd);
	return 0;
}

/*! \fn int Gpio::SetDir(bool out)
 * 	\brief Set the gpio pin direction.
 * 	\param out True if wants to set the pin as an output. False otherwise.
 * 	\return 0 on success, -1 on fail.
 */
int Gpio::SetDir(bool out)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/direction", _pinNum);

	fd = open(buf, O_WRONLY);
	if (fd < 0)
	{
		perror("gpio/direction");
		return fd;
	}
	
	if( out )
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	_isOut = out;
	return 0;
}

/*! \fn int Gpio::SetValue(GpioVal value)
 * 	\brief Set the gpio pin value when the gpio pin is in output mode.
 * 	\param value HIGH or LOW.
 * 	\return 0 on success, -1 on fail.
 */
int Gpio::SetValue(GpioVal value)
{
	if( _isOut && _curVal != value )
	{
		if( _fd < 0 ) OpenFd();
		if( _fd < 0 ) return _fd;

		switch(value)
		{
			case HIGH:
				write(_fd, "1", 2);
				break;
			case LOW:
				write(_fd, "0", 2);
				break;
			default:
				break;
		}

		_curVal = value;
	}
	return 0;
}

/*! \fn int Gpio::GetValue(GpioVal & value)
 * 	\brief Get the gpio pin value when the pin is in input mode.
 * 	\param value The value HIGH or LOW of the gpio input reading.
 * 	\return 0 on success, -1 on fail.
 */
int Gpio::GetValue(GpioVal & value)
{
	if( !_isOut )
	{
		if( _fd < 0 ) OpenFd();
		if( _fd < 0 ) return _fd;

		char ch;

		read(_fd, &ch, 1);

		if (ch != '0')
			value = HIGH;
		else
			value = LOW;

		_curVal = value;
	}
	return 0;
}

/*! \fn int Gpio::SetEdge(GpioEdge edge)
 * 	\brief Set the trigger edge when the gpio is in input mode.
 * 	\param edge RISING, FALLING, or BOTH.
 * 	\return 0 on success, -1 on fail.
 */
int Gpio::SetEdge(GpioEdge edge)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", _pinNum);

	fd = open(buf, O_WRONLY);
	if (fd < 0)
	{
		perror("gpio/setEdge");
		return fd;
	}

	switch(edge)
	{
		case RISING:
			write(fd, "rising", 7);
			break;
		case FALLING:
			write(fd, "falling", 8);
			break;
		case BOTH:
			write(fd, "both", 5);
			break;
		default:
			break;
	}
	
	close(fd);
	return 0;
}

/*! \fn void Gpio::OpenFd()
 * 	\brief Initialized the file descriptor for the gpio pin.
 */
void Gpio::OpenFd()
{
	int len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", _pinNum);
	
	if( _fd >= 0 ) close(_fd);
	
	if( _isOut )
		_fd = open(buf, O_RDWR);
	else
		_fd = open(buf, O_RDONLY);

	if ( _fd < 0 )
	{
		perror("gpio/fdOpen");
	}
}

/*! \fn void Gpio::CloseFd()
 * 	\brief Close the file descriptor for this gpio pin.
 */
void Gpio::CloseFd()
{
	close(_fd);
}

/*! \fn int Gpio::Poll(int usec, struct timespec & timeOfInterrupt)
 * 	\brief Polling the gpio input and record the time when the trigger occurred.
 *  \param usec The timeout periold for polling in microsecond.
 * 	\param timeOfInterrupt The recorded time when the triggering occurred.
 * 	\return 0 if the timeout occurred, > 0 if triggering happened, -1 if something went wrong.
 */
int Gpio::Poll(int usec, struct timespec & timeOfInterrupt)
{
	int len;
	char buf[MAX_BUF];
	struct pollfd fdset[1];

	memset((void*) fdset, 0, sizeof(fdset));
	fdset[0].fd = _fd;
	fdset[0].events = POLLPRI;

	int ret = poll(fdset, 1, usec);
	clock_gettime(CLOCK_REALTIME, &timeOfInterrupt);
	if (ret < 0)
	{
		perror("\npoll() failed!\n");
		return -1;
	}
	else if (ret > 0)
	{
		if (fdset[0].revents & POLLPRI)
		{
			len = read(fdset[0].fd, buf, MAX_BUF);
			return ret;
		}
	}
	// timeout
	return 0;
}
