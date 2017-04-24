#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include "Packet.h"
#include <termios.h>
#include <fcntl.h>
#include <poll.h>

#include "Camera.h"

#define CREATE_PORT 8888
#define VIDEO_PORT 8855
#define ARTAG_PORT 8844

using namespace std;

bool showDebugMsg = true;
pthread_cond_t endCondition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t endMutex = PTHREAD_MUTEX_INITIALIZER;
bool isInit = false;
bool isEnding = false;
unsigned long connectedHost = 0;

int remoteSock;
struct sockaddr_in remoteVideo;
struct sockaddr_in remoteARtag;
struct sockaddr_in remoteCreate;

Camera * camera;
pthread_t sensorThread;

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void debugMsg(const char *func, const char *msg)
{
	if (showDebugMsg)	printf("[%s	] %s\n", func, msg);
}

void* RunARtagVideo(void* arg)
{
	camera->StreamARtagVideo();
	pthread_exit(NULL);
}

void* StreamSensorData(void* arg)
{
	debugMsg(__func__, "Start streaming sensor data ...");
	unsigned long addr = *((unsigned long*) arg);
	printf("[%s	] addr: %d\n", __func__, (int)addr);
	isInit = true;
	pthread_t artagThread;
	printf("ARtag Thread: %d.\n", 
		pthread_create(&artagThread, NULL, RunARtagVideo, NULL));
	camera = new Camera(remoteSock, remoteVideo, remoteARtag);
	while(1)
	{
		pthread_mutex_lock( &endMutex );
		if (isEnding)
		{
			isEnding = false;
			pthread_mutex_unlock( &endMutex );
			connectedHost = 0;
			camera->QuitMainLoop();
			break;
		}
		pthread_mutex_unlock( &endMutex );
		sleep(5);
		// run artag
		// run sonar
		// run odometry
	}
	debugMsg(__func__, "End of streaming sensor data");
	isInit = false;
	
	pthread_join(artagThread, NULL);
	delete camera;
	pthread_exit(NULL);
}

void MakeConnection(Packet & packet)
{
	if (isInit)
	{
		debugMsg(__func__, "Connection is already occupied.");
		return;
	}
	remoteSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (remoteSock < 0) error("socket");

	remoteVideo.sin_family = AF_INET;
	remoteVideo.sin_addr.s_addr = packet.addr.s_addr;
	remoteVideo.sin_port = htons(VIDEO_PORT);

	remoteARtag.sin_family = AF_INET;
	remoteARtag.sin_addr.s_addr = packet.addr.s_addr;
	remoteARtag.sin_port = htons(ARTAG_PORT);

	remoteCreate.sin_family = AF_INET;
	remoteCreate.sin_addr.s_addr = packet.addr.s_addr;
	remoteCreate.sin_port = htons(CREATE_PORT);

	connectedHost = packet.addr.s_addr;
	
	printf("Sensor Thread: %d.\n", 
		pthread_create(&sensorThread, NULL, StreamSensorData, (void*)&packet.addr.s_addr));
}

void ProcessPackets(Packet & packet)
{
	switch(packet.type)
	{
		case INIT:
			debugMsg(__func__, "======= packet received, type: INIT");
			MakeConnection(packet);
			break;
		case END:
			debugMsg(__func__, "======= packet received, type: END");
			if (!isInit)
				debugMsg(__func__, "No connection was initialized.");
			else
			{
				pthread_mutex_lock( &endMutex );
				isEnding = true;
				pthread_mutex_unlock( &endMutex );
				pthread_join(sensorThread, NULL);
			}
			break;
		case CTRL:
			debugMsg(__func__, "======= packet received, type: CTRL");
			if (connectedHost == packet.addr.s_addr)
			{
			}
			else
			{
				debugMsg(__func__, "There is no connection made with this client, please INIT first");
			}
			break;
		case DATA:
			debugMsg(__func__, "======= packet received, type: DATA");
			break;
		case ERROR:
			debugMsg(__func__, "======= packet received, type: ERROR");
			break;
		case SHUTDOWN:
			debugMsg(__func__, "======= packet received, type: SHUTDOWN");
			pthread_mutex_lock( &endMutex );
			isEnding = true;
			pthread_mutex_unlock( &endMutex );
			break;
		default:
			debugMsg(__func__, "======= packet received, type: UNKNOWN");
			packet.type = UNKNOWN;
			break;
	}
}

void* ListenMessage(void* arg)
{
	int sock, bufLength;
	socklen_t serverlen, fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	char buf[MAXPACKETSIZE];
	Packet packet;

	// initialize udp listener
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) error("Opening socket");
	serverlen = sizeof(server);
	bzero(&server, serverlen);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi((char*)arg));
	if (bind(sock, (struct sockaddr *)&server, serverlen) < 0) error("binding");
	fromlen = sizeof(struct sockaddr_in);

	debugMsg(__func__, "Waiting for INIT message ...");
	while(1)
	{
		bzero(&buf, sizeof(buf));
		bzero(&packet, sizeof(Packet));
		packet.type = UNKNOWN;
		bufLength = recvfrom(sock, buf, MAXPACKETSIZE, 
				0, (struct sockaddr *)&from, &fromlen);
		if (bufLength < 0) error("recvfrom");

		memcpy((unsigned char*)&packet, buf, 256);
		packet.addr = from.sin_addr;
		packet.port = from.sin_port;
		ProcessPackets(packet);
		if (packet.type == END) debugMsg(__func__, "Waiting for INIT message ...");
		if (packet.type == SHUTDOWN) break;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	// loading input parameters
	if (argc < 2)
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(0);
	}
	if (argc > 2)
	{
		showDebugMsg = (strcmp(argv[2],"hideDebug") == 0)?false:showDebugMsg;
	}


	pthread_t listenerThread;
	pthread_cond_init(&endCondition, NULL);
	printf("Listener Thread: %d.\n", 
		pthread_create(&listenerThread, NULL, ListenMessage, (void*)argv[1]));

	pthread_join(listenerThread, NULL);
	pthread_cond_destroy(&endCondition);
	// start sensor data thread
/*	StreamSensorData();
	// start message listening thread
	ListenMessage();*/

	// return to idle when receive a finish msg 

	// or restart when receive another init msg

	return 0;
}
