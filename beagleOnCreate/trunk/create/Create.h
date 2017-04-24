#ifndef CREATE_H
#define CREATE_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "pthread.h"
#include "../Packet.h"

/*! \file Create.h */

/*! The tcp port number for direct control of the iRobot Create. */
#define CREATE_PORT 8865

class Create
{
public:
	Create();
	~Create();
	
	int InitSerial();
	void CloseSerial();
	int RunSerialHandler();
	int RunTCPListener();
	
	/*! Flag to get the Create class ready to quit. */
	bool isEnding;

private:
	int _fd;
	int _sock;
	int _bufLength;
	char _buf[MAXPACKETSIZE];

	pthread_mutex_t _serialMutex;
};

#endif
