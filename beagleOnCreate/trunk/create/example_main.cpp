#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>

#include "Create.h"
#include "../Packet.h"

#define BEAGLE_PORT 8866

Create* create;

void* CreateSerialHandler(void* arg)
{
	return (void*) create->RunSerialHandler();
}

void* CreateTCPListener(void* arg)
{
	return (void*) create->RunTCPListener();
}

void StartListening(Packet & packet)
{
	create = new Create();
	pthread_t createSerialThread;
	printf("iRobot Create SerialListner Thread: %d.\n", 
		pthread_create(&createSerialThread, NULL, CreateSerialHandler, NULL));
	pthread_t createTCPThread;
	printf("iRobot Create TCPListner Thread: %d.\n", 
		pthread_create(&createTCPThread, NULL, CreateTCPListener, NULL));
}

int main(int argc, char *argv[])
{
	int sock, clientsock, bufLength;
	socklen_t serverlen, fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	char buf[MAXPACKETSIZE];
	Packet packet;

	// initialize udp listener
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) printf("ERROR: Opening socket\n");
	serverlen = sizeof(server);
	bzero(&server, serverlen);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(BEAGLE_PORT);
	if (bind(sock, (struct sockaddr *)&server, serverlen) < 0) 
		printf("ERROR: binding\n");
	fromlen = sizeof(struct sockaddr_in);

	printf("Waiting for INIT message ...\n");
	if (listen(sock, 1) < 0) return -1;
	if ((clientsock = accept(sock, (struct sockaddr *) &from, &fromlen)) < 0) return -1;

	while(1)
	{
		bzero(&buf, sizeof(buf));
		bzero(&packet, sizeof(Packet));
		packet.type = UNKNOWN;
		bufLength = recvfrom(clientsock, buf, MAXPACKETSIZE, 
				0, (struct sockaddr *)&from, &fromlen);
		if (bufLength == 0) continue;
		if (bufLength < 0) printf("ERROR: recvfrom\n");

		memcpy((unsigned char*)&packet, buf, 256);
		packet.addr = from.sin_addr;
		packet.port = from.sin_port;
		if (packet.type == INIT)
		{
			StartListening(packet);
		}
		if (packet.type == END) 
		{
			printf("Waiting for INIT message ...\n");
		}
		if (packet.type == SHUTDOWN) 
		{
			create->isEnding = true;
			break;
		}
	}
	return 0;
}
