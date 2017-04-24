#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <termios.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

#include "Create.h"

/*! \file Create.cpp */

/*! The serial port path in the sys_fs. */
#define CREATE_SERIAL_PORT "/dev/ttyUSB0"
/*! The serial port baudrate for serial communication with iRobot Create. */
#define CREATE_SERIAL_BRATE B57600

/*!
 * 	\class Create Create.h "Create.h"
 *	\brief This class handles all serial communication with iRobot Create.
 */

/*! \fn Create::Create(int sock, struct sockaddr_in & createPort, unsigned long connectedHost)
 *  \brief A constructor for Create class. The serial communication is initialized here.
 */
Create::Create()
{
	_fd = -1;
	isEnding = false;
	pthread_mutex_init(&_serialMutex, NULL);
	_bufLength = 0;
	InitSerial();
}

/*! \fn Create::~Create()
 * 	\brief A destructor for Create class. The serial communication is cleaned up here.
 */
Create::~Create()
{
	pthread_mutex_destroy(&_serialMutex);
	CloseSerial();
}

/*! \fn int Create::InitSerial()
 * 	\brief To initialize the serial communication.
 * 	\return fd, the file descriptor on success, -1 on fail.
 */
int Create::InitSerial()
{
	_fd = open(CREATE_SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);

	if(_fd == -1) // if open is unsuccessful
	{
		printf("Unable to open %s.\n", CREATE_SERIAL_PORT);
		return -1;
	}
	else
	{
		fcntl(_fd, F_SETFL, 0);
		printf("Create serial port opened.\n");
	}

	// configure port
	struct termios portSettings;
	tcgetattr( _fd, &portSettings);
		
	if (cfsetispeed(&portSettings, CREATE_SERIAL_BRATE) != 0)
		printf("Failed setting cfsetispeed\n");
	if (cfsetospeed(&portSettings, CREATE_SERIAL_BRATE) != 0)
		printf("Failed setting cfsetospeed\n");
		
	// set no parity, stop bits, databits
	portSettings.c_cflag &= ~PARENB;
	portSettings.c_cflag &= ~CSTOPB;
	portSettings.c_cflag &= ~CSIZE;
	portSettings.c_cflag |= CS8;
	// make raw mode
	cfmakeraw(&portSettings);

	if (tcsetattr(_fd, TCSANOW, &portSettings) != 0)
		printf("Failed pushing portSettings\n");
	
	return _fd;
}

/*! \fn void Create::CloseSerial()
 * 	\brief Close the serial communication.
 */
void Create::CloseSerial()
{
	close(_fd);
}

/*! \fn int Create::RunSerialHandler()
 *  \brief The serial handler to listen anything coming from the iRobot Create or write serial to it.
 * 	\return 0 on success, -1 on fail.
 */
int Create::RunSerialHandler()
{
	char buf[MAXPACKETSIZE];
	int ret;
	int bufLength;
	int            max_fd;
	fd_set         input, output;
	struct timeval timeout;
	
	if (_fd == -1)
	{
		printf("ERROR: _fd is not initialized\n");
		return -1;
	}

	while(1)
	{
		usleep(100);
		if(isEnding)
			break;
			
		/* Initialize the input set */
		FD_ZERO(&input);
		FD_ZERO(&output);
		FD_SET(_fd, &input);
		FD_SET(_fd, &output);
		max_fd = _fd + 1;
		
		/* Initialize the timeout structure */
		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;

		/* Do the select */
		ret = select(max_fd, &input, &output, NULL, &timeout);
		//ret = select(max_fd, &input, NULL, NULL, NULL);

		/* See if there was an error */
		if (ret < 0)
			printf("ERROR: select failed\n");
		else if (ret != 0)
		{
			/* We have input */
			if (FD_ISSET(_fd, &input))
			{
				bufLength = read(_fd, buf, MAXPACKETSIZE);
				if (bufLength == -1)
				{
					printf("ERROR: read\n");
					continue;
				}

				if (send(_sock, buf, bufLength, 0) != bufLength)
					printf("ERROR: send\n");

				printf("Received from Create: \n");
				for (int i = 0; i < bufLength; i++)
				{	
					printf("%i ", int(buf[i]));
				}
				printf("\n");
			}

			if (FD_ISSET(_fd, &output))
			{
				pthread_mutex_lock(&_serialMutex);
				bufLength = _bufLength;
				_bufLength = 0;
				if (bufLength > 0)
				{
					bzero(&buf, sizeof(buf));
					memcpy(buf, _buf, bufLength);
				}
				pthread_mutex_unlock(&_serialMutex);
				if (bufLength <= 0)
					continue;

				ret = write(_fd, buf, bufLength);
				if (ret == -1)
				{
					printf("ERROR: write error occurred.\n");
					continue;
				}
				printf("Sending to Create: \n\t\t");
				for (int i = 0; i < bufLength; i++)
				{
					printf("%i ", int(buf[i]));
				}
				printf("\n");
			}

		}

		fflush(stdout);
	}
	return 0;
}

/*! \fn int Create::RunTCPListener(int & sock)
 * 	\brief The main loop of the TCP listener.
 * 	\return 0 on success, -1 on fail.
 */
int Create::RunTCPListener()
{
	int sock, clientsock, bufLength;
	socklen_t serverlen, fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	char buf[MAXPACKETSIZE];

	// initialize tcp listener
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) 
	{
		printf("ERROR: Opening socket\n");
		return -1;
	}
	serverlen = sizeof(server);
	bzero(&server, serverlen);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(CREATE_PORT);
	if (bind(sock, (struct sockaddr *)&server, serverlen) < 0)
	{
		printf("ERROR: binding\n");
		return -1;
	}
	fromlen = sizeof(struct sockaddr_in);

	printf("Ready to listen to Create message ...\n");
	if (listen(sock, 1) < 0) 
	{
		printf("failed to listen on Create socket\n");
		return -1;
	}
	clientsock = accept(sock, (struct sockaddr *) &from, &fromlen);
	if (clientsock < 0)
	{
		printf("failed to accept client connection on Create socket\n");
	       return -1;	
	}
	_sock = clientsock;
	while(1)
	{
		if (isEnding)
		{
			printf("RunUDPListener received Ending flag\n");
			break;
		}
		
		bzero(&buf, sizeof(buf));
		bufLength = recvfrom(clientsock, buf, MAXPACKETSIZE, 
				0, (struct sockaddr *)&from, &fromlen);
				
		if (bufLength == 0)
		{
			usleep(1000);
			continue;
		}

		pthread_mutex_lock(&_serialMutex);
		_bufLength = bufLength;
		if (bufLength > 0)
		{
			bzero(&_buf, sizeof(_buf));
			memcpy(_buf, buf, bufLength);
		}
		pthread_mutex_unlock(&_serialMutex);

		if (bufLength < 0) 
		{
			printf("ERROR: recvfrom %d\n", errno);
		}
	}
	usleep(2000000);
	close(sock);
	//CloseSerial();
	printf("Ending RunUDPListener \n");
	return 0;
}
