//
// Created by awadell on 4/26/2017.
//

#include "processMsg.h"
#include "msg_id.h"

// Process MSG
int processMsg(char sendBuff[], const char* msg) {
    // Report Message
    printf("\tMSG: %s\n", msg);

    // Message Response
    int startIndex = 8;
    char resp[MAX_MSG_SIZE];
    int status;

    if (strncmp(msg, SET_WHEEL_SPEED, 3) == 0) {
        strncpy(resp, "25,26", 5);
        status = 1;
    } else if (strncmp(msg, GET_ODOMETRY, 3) == 0) {
        strncpy(resp, "Goodbye", 7);
        status = -1;
    } else if (strncmp(msg, GET_RANGE_READING, 3) == 0) {
        strncpy(resp, "Goodbye", 7);
        status = -1;
    } else if (strncmp(msg, GET_IMU_READING, 3) == 0) {
        strncpy(resp, "Goodbye", 7);
        status = -1;
    } else if (strncmp(msg, SHUTDOWN_COMMS, 3) == 0) {
        strncpy(resp, "Goodbye", 7);
        status = -1;
    }
    else {
        // Echo message
        strcpy(resp, msg);
        status = 1;
    }

    // Check that response is not too long
    size_t respLen = strlen(resp);
    if (respLen > (MAX_MSG_SIZE - startIndex - 1)) {
        respLen = MAX_MSG_SIZE - startIndex - 1;
    }

    // Copy response into send buffer
    strncpy(sendBuff+startIndex, resp, respLen);

    return status;
}