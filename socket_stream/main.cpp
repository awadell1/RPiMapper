/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <chrono>

// Header Files
#include "processMsg.h"

// Define Port to listen on
#define RPI_PORT 1611



void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    // Create variables to store connection information
    int sockfd;         // sockfd:      Server Socket File
    int clientfd;       // clientfd:    Socket to client
    int portno;         // portno:      The port the server listens on
    socklen_t clilen;   // clilen:      The length of the client address

    // Open Socket
    struct sockaddr_in serv_addr, cli_addr;
    int option =1;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (sockfd < 0) error("ERROR opening socket");

    // Set Address and Port Number
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(RPI_PORT);

    // Bind Port settings to socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // Wait for a client to connect
    listen(sockfd, 1);
    clilen = sizeof(cli_addr);
    printf("Waiting for client to connect...\n");
    clientfd = -1;
    while (clientfd < 0){
        clientfd = accept(sockfd,
                          (struct sockaddr *) &cli_addr,
                          &clilen);
        // Check connection status
        if (clientfd < 0) printf("ERROR: Unable to accept client\n");
    }
    printf("SUCCESS: Client Connected\n");

    // Start Connection Timer
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

    // Create Buffer for reading and sending messages
    char sendBuff[MAX_MSG_SIZE];
    char recvBuff[MAX_MSG_SIZE];

    bool isAlive = true;    // Flag for client is still connected
    _ssize_t n;             // Length of received/sent message
    int msgStatus = -1;     // Response from processMsg
    double respTime;        // Stores the time at which the msg is responded too (roughly), in seconds since start
    int errorCount = 0;
    while (isAlive){
        // Clear the buffers
        bzero(sendBuff, sizeof(sendBuff));
        bzero(recvBuff, sizeof(recvBuff));

        // Read Message from client
        n = read(clientfd, recvBuff, sizeof(recvBuff)-1);

        // Check for receive error
        if (n<0){
            printf("ERROR: Reading message from socket\n");
            errorCount++;
        } else {
            // Display message
            printf("RECV: %s\n", recvBuff);

            // Append Msg with Receive time
            std::chrono::duration<double> elapsed_seconds = std::chrono::steady_clock::now()  - startTime;
            respTime = elapsed_seconds.count();
            memcpy(&sendBuff, &respTime, sizeof(respTime));

            // Process message
            msgStatus = processMsg(sendBuff, recvBuff);

            // Report Success
            n = write(clientfd, sendBuff, sizeof(sendBuff));
            if (n < 0) printf("ERROR: Unable to write to client");

            // Check for Exit Flag
            if(msgStatus < 0) isAlive = false;
        }

        // Check for too many errors
        if(errorCount > 20) isAlive = false;
    }

    // Close the connection
    close(clientfd);
    close(sockfd);

    return 0;
}