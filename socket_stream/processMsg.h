//
// Created by awadell on 4/26/2017.
//

#ifndef SOCKET_STREAM_PROCESSMSG_H
#define SOCKET_STREAM_PROCESSMSG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Set the max size of messages to be sent
#define MAX_MSG_SIZE 1026

// Declare Functions
int processMsg(char sendBuff[], const char* msg);

#endif //SOCKET_STREAM_PROCESSMSG_H
