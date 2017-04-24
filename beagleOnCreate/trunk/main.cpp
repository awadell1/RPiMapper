/**
@mainpage

Beagle on Create is a project that utilizes Beagleboard with a robotic development platform iRobot Create. The Beagleboard serves as the sensor interface as well as teleoperation device for the Create. The sensors including an USB webcam and 3 sonars.

@author Chuck Yang ty244
@date 1/15/2012
@version 1.0
*/

/*! \file main.cpp
 */

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
#include <sys/resource.h>

#include "Packet.h"
#include "control/Control.h"
#include "camera/Camera.h"
#include "create/Create.h"
#include "sonar/Sonar.h"

/*! The udp port number for talking with this program. */
#define BEAGLE_PORT 8866

/*! The gpio pin number that sonar 1 is connected to. */
#define SONAR_GPIO1 136
/*! The gpio pin number that sonar 2 is connected to. */
#define SONAR_GPIO2 135
/*! The gpio pin number that sonar 3 is connected to. */
#define SONAR_GPIO3 134

/*! Max connection requests */
#define MAXPENDING 1

using namespace std;

/*! Flag to show debug message. */
bool showDebugMsg = true;
/*! The end condition to signal quitting of the program. */
pthread_cond_t endCondition = PTHREAD_COND_INITIALIZER;
/*! The mutex for the end condition. */
pthread_mutex_t endMutex = PTHREAD_MUTEX_INITIALIZER;
/*! Flag to indicate that the program has initialized. */
bool isInit = false;
/*! Flag to get ready to quit the program. */
bool isEnding = false;
/*! The remote UDP socket file descriptor. */
int remoteSockUDP;
/*! The sockaddr_in struct that is associated with the video udp port. */
struct sockaddr_in remoteVideo;
/*! The sockaddr_in struct that is associated with the ARtag udp port. */
struct sockaddr_in remoteARtag;
/*! The sockaddr_in struct that is associated with the Sonar udp port. */
struct sockaddr_in remoteSonar;
/*! The sensor thread for streaming all sensor data to the MATLAB. */
pthread_t sensorThread;

/*! The camera object for ARtag detection. */
Camera * camera;
/*! The create object for iRobot Create teleop. */
Create * create;
/*! The sonar object for sonar 1. */
Sonar * sonar1;
/*! The sonar object for sonar 2. */
Sonar * sonar2;
/*! The sonar object for sonar 3. */
Sonar * sonar3;

/*! 
 * 	\brief The error printing function.
 * 	\param msg the msg to print out.
 */
void error(const char *msg)
{
	perror(msg);
	exit(0);
}

/*! 
 * 	\brief A function to print debug message.
 * 	\param func the function name. (put __FUNC__ macro)
 * 	\param msg any message you desire.
 */
void debugMsg(const char *func, const char *msg)
{
	if (showDebugMsg)	printf("[%s	] %s\n", func, msg);
}

/*! 
 * 	\brief Handles the control for the universal gripper.
 * 	\param packet The Packet received.
 */
void HandleControls(Packet & packet)
{
	int ctrlType = packet.u.ctrl.data[0];
	switch (ctrlType)
	{
		case 0:
			printf("drop \n");
			system(DROPBASH);
			break;
		case 1:
			printf("pickup \n");
			system(PICKUPBASH);
			break;
		case 2:
			printf("resetarm \n");
			system(RESETBASH);
			break;
		case 3:
			printf("video broadcast on \n");
			camera->SetVideoBroadcast(true);
			break;
		case 4:
			printf("video broadcast off \n");
			camera->SetVideoBroadcast(false);
			break;
		default:
			break;
	}	

	printf("packet data: %d\n", ctrlType);
}

/*!
 * 	\brief Start running ARtag detection.
 * 	\param arg Any possible argument that is passed in with this thread.
 */
void* RunARtagVideo(void* arg)
{
	camera->StreamARtagVideo();
	pthread_exit(NULL);
}

/*!
 * 	\brief Start running serial handler for iRobot Create.
 * 	\param arg Any possible argument that is passed in with this thread.
 */
void* CreateSerialHandler(void* arg)
{
	create->RunSerialHandler();
	pthread_exit(NULL);
}

/*!
 * 	\brief Start running TCP listener for iRobot Create.
 * 	\param arg Any possible argument that is passed in with this thread.
 */
void* CreateTCPListener(void* arg)
{
	create->RunTCPListener();
	pthread_exit(NULL);
}

/*!
 * 	\brief Start running sonar measurement and send over the measurement through udp.
 * 	\param arg Any possible argument that is passed in with this thread.
 */
void* SonarSender(void* arg)
{
	float dist1 = -1.f;
	float dist2 = -1.f;
	float dist3 = -1.f;
	Packet packet;
	packet.type = SONAR;
	
	while(1)
	{
		if (sonar1->isEnding)
			break;
		
		dist1 = sonar1->Run();	
		usleep(SONAR_WAIT_TIME);
		dist2 = sonar2->Run();	
		usleep(SONAR_WAIT_TIME);
		dist3 = sonar3->Run();			
		usleep(SONAR_WAIT_TIME);
		// send out dist
		packet.u.sonar.dist1 = dist1;
		packet.u.sonar.dist2 = dist2;
		packet.u.sonar.dist3 = dist3;
		if (sendto(remoteSockUDP, (unsigned char*)&packet, SONAR_PACKET_SIZE, 0, (const struct sockaddr *)&remoteSonar, sizeof(struct sockaddr_in)) < 0) printf("sendto\n");
		usleep(SONAR_MEASURE_RATE);
	}
	pthread_exit(NULL);
}

/*!
 * 	\brief Start running dumb load which peg the cpu load at 99%.
 * 	\param arg Any possible argument that is passed in with this thread.
 */
void* Dumbload(void* arg)
{
	while(1)
	{
	}
	pthread_exit(NULL);
}

/*!
 * 	\brief Start streaming all the sensor data including sonar, artag, and create.
 * 	\param arg Any possible argument that is passed in with this thread.
 */
void* StreamSensorData(void* arg)
{
	debugMsg(__func__, "Start streaming sensor data ...");
	isInit = true;
	
	camera = new Camera(remoteSockUDP, remoteVideo, remoteARtag);
	create = new Create();
	sonar1 = new Sonar(SONAR_GPIO1);
	sonar2 = new Sonar(SONAR_GPIO2);
	sonar3 = new Sonar(SONAR_GPIO3);
	
	// run create control	
	pthread_t createSerialThread;
	printf("iRobot Create SerialHandler Thread: %d.\n", 
		pthread_create(&createSerialThread, NULL, CreateSerialHandler, NULL));
		
	pthread_t createTCPThread;
	printf("iRobot Create TCPListner Thread: %d.\n", 
		pthread_create(&createTCPThread, NULL, CreateTCPListener, NULL));
		
	usleep(1000);

	// run camera
	pthread_t cameraThread;
	printf("Camera Thread: %d.\n", 
		pthread_create(&cameraThread, NULL, RunARtagVideo, NULL));
		
	// run sonar
	pthread_t sonarThread;
	printf("Sonar Thread: %d.\n", 
		pthread_create(&sonarThread, NULL, SonarSender, NULL));

	// run dumbload
	#if 0
	pthread_t dumbloadThread;
	printf("dumbload Thread: %d.\n", 
		pthread_create(&dumbloadThread, NULL, Dumbload, NULL));
	#endif
	
	while(1)
	{
		pthread_mutex_lock( &endMutex );
		if (isEnding)
		{
			isEnding = false;
			pthread_mutex_unlock( &endMutex );
			camera->QuitMainLoop();
			create->isEnding = true;
			sonar1->isEnding = true;
			break;
		}
		pthread_mutex_unlock( &endMutex );
		sleep(2);
	}
	debugMsg(__func__, "Waiting for threads halt...");
	isInit = false;
	
	pthread_join(cameraThread, NULL);
	debugMsg(__func__, "cameraThread halted");
	pthread_join(createSerialThread, NULL);
	debugMsg(__func__, "createSerialThread halted");
	pthread_join(createTCPThread, NULL);
	debugMsg(__func__, "createTCPThread halted");
	pthread_join(sonarThread, NULL);
	debugMsg(__func__, "sonarThread halted");
	delete camera;
	delete create;
	delete sonar1;
	delete sonar2;
	delete sonar3;
	debugMsg(__func__, "End of streaming sensor data.");
	pthread_exit(NULL);
}

/*!
 * 	\brief Making the connection after the init packet is received. Start streaming sensor data.
 * 	\param packet The Packet received.
 */
void MakeConnection(Packet & packet)
{
	if (isInit)
	{
		debugMsg(__func__, "Connection is already occupied.");
		return;
	}
	remoteSockUDP = socket(AF_INET, SOCK_DGRAM, 0);
	if (remoteSockUDP < 0) error("socket UDP");

	remoteVideo.sin_family = AF_INET;
	remoteVideo.sin_addr.s_addr = packet.addr.s_addr;
	remoteVideo.sin_port = htons(VIDEO_PORT);

	remoteARtag.sin_family = AF_INET;
	remoteARtag.sin_addr.s_addr = packet.addr.s_addr;
	remoteARtag.sin_port = htons(ARTAG_PORT);

	remoteSonar.sin_family = AF_INET;
	remoteSonar.sin_addr.s_addr = packet.addr.s_addr;
	remoteSonar.sin_port = htons(SONAR_PORT);

	printf("Sensor Thread: %d.\n", 
		pthread_create(&sensorThread, NULL, StreamSensorData, NULL));
}

/*!
 * 	\brief Process the Packet that is received based on its PacketType.
 * 	\param packet The Packet received.
 */
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
			}
			break;
		case CTRL:
			debugMsg(__func__, "======= packet received, type: CTRL");
			HandleControls(packet);
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

/*!
 * 	\brief Start listening to any packet coming in from the remote host.
 * 	\param arg Any possible argument that is passed in with this thread.
 */
void* ListenMessage(void* arg)
{
	int sock, clientsock, bufLength;
	socklen_t serverlen, fromlen;
	struct sockaddr_in server;
	struct sockaddr_in from;
	char buf[MAXPACKETSIZE];
	Packet packet;

	// initialize udp listener
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) error("Opening socket");
	serverlen = sizeof(server);
	bzero(&server, serverlen);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(BEAGLE_PORT);
	if (bind(sock, (struct sockaddr *)&server, serverlen) < 0) error("binding");
	fromlen = sizeof(struct sockaddr_in);

	// Listen on the server socket
	debugMsg(__func__, "Waiting for INIT message ...");
	if (listen(sock, MAXPENDING) < 0) error("listen");

	if ((clientsock = accept(sock, (struct sockaddr *) &from, &fromlen)) < 0)
		error("failed to accept client connection");
	while(1)
	{
		bzero(&buf, sizeof(buf));
		bzero(&packet, sizeof(Packet));
		packet.type = UNKNOWN;

		bufLength = recvfrom(clientsock, buf, MAXPACKETSIZE, 
				0, (struct sockaddr *)&from, &fromlen);

		if (bufLength == 0) 
		{
			usleep(1000);
			continue;
		}
		if (bufLength < 0) error("recvfrom");

		memcpy((unsigned char*)&packet, buf, 256);
		packet.addr = from.sin_addr;
		packet.port = from.sin_port;
		ProcessPackets(packet);
		if (packet.type == END) debugMsg(__func__, "Waiting for INIT message ...");
		if (packet.type == SHUTDOWN) break;
	}
	pthread_join(sensorThread, NULL);
	usleep(2000000);
	close(sock);

	return 0;
}

/*!
 * 	\brief The "main" function, start the listener thread.
 * 	\param argc The argument size.
 * 	\param argv The argument value.
 * 	\return 0 when done.
 */
int main(int argc, char *argv[])
{
	// loading input parameters
	if (argc > 1)
	{
		showDebugMsg = (strcmp(argv[2],"hideDebug") == 0)?false:showDebugMsg;
	}

	pthread_t listenerThread;
	pthread_cond_init(&endCondition, NULL);
	printf("Listener Thread: %d.\n", 
		pthread_create(&listenerThread, NULL, ListenMessage, (void*)argv[1]));

	pthread_join(listenerThread, NULL);
	pthread_cond_destroy(&endCondition);

	return 0;
}
