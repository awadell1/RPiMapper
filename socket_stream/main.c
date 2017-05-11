/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

// Set the Wheel Speeds
#define SET_WHEEL_SPEED "SWV"

// Get Odometry Measurement
#define GET_ODOMETRY "GOM"

// Get Range Measurement
#define GET_RANGE_READING "GRM"

// Get IMU Measurement
#define GET_IMU_READING "GIR"

// Close Connection
#define SHUTDOWN_COMMS "SDC"

// Define Port to listen on
#define RPI_PORT 1618

// Set the max size of messages to be sent
#define MAX_MSG_SIZE 1026

// Set the number of sonars
#define NUM_SONAR 8

// Set the I2C Addresses
#define ARDUINO_I2C 0x05
#define MAGNO_I2C 0x1E
#define Gyro_I2c 0x6B

// Global variables
int i2c_bus = 0;        // File handle for i2c bus
int sockfd = 0;         // File handle for server socket
int clientfd = 0;       // File handle for client socket
 
// Declare Functions
void sighandler(int signum);

int processMsg(char sendBuff[], const char* msg);

int pollArduino(char buffer[], const char msg[], const int buffSize);

int openI2C();

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    // Setup Signal Handler
    signal(SIGINT, sighandler);

    // Setup I2C Connection to Arduino
    if (openI2C() != 1) {return -1;}
   
    // Create variables to store connection information
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
    time_t startTime = time(NULL);

    // Create Buffer for reading and sending messages
    char sendBuff[MAX_MSG_SIZE];
    char recvBuff[MAX_MSG_SIZE];

    int isAlive = 1;    // Flag for client is still connected
    size_t n;             // Length of received/sent message
    int msgStatus = -1;     // Response from processMsg
    double respTime;        // Stores the time at which the msg is responded too (roughly), in seconds since start
    int errorCount = 0;
    while (isAlive==1){
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

            // Prepend Msg with Receive time
            respTime = time(NULL) - startTime;
            sprintf(sendBuff, "%f ", respTime);

            // Process message
            msgStatus = processMsg(sendBuff, recvBuff);

            // Report Success
            n = write(clientfd, sendBuff, sizeof(sendBuff));
            if (n < 0) printf("ERROR: Unable to write to client");

            // Check for Exit Flag
            if(msgStatus < 0) isAlive = 0;
        }

        // Check for too many errors
        if(errorCount > 20) isAlive = 0;
    }

    // Close the connection
    close(clientfd);
    close(sockfd);

    return 0;
}

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
        // Request Sonar Readings from Arduino
        int n = 16;
        char sonarData[n];
        char cmd[n];
        int i;
        for(i=0; i < NUM_SONAR; i++){
            // Construct Command 
            sprintf(cmd, "s%d", i);

            // Poll Arduino
            int dataSize = pollArduino(sonarData, cmd, n);

            // Append to sendBuff
            strcat(sendBuff, sonarData);
            strcat(sendBuff, " ");
        }

        return status;
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

int pollArduino(char buffer[], const char msg[], const int buffSize){
    // Attempt to connect to Arduino
    if (ioctl(i2c_bus,I2C_SLAVE, ARDUINO_I2C) < 0) {
        printf("Failed to acquire bus access and/or talk to Arduino.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        return -1;
    }

    // Send msg to arduino
    int n = write(i2c_bus, msg, strlen(msg));
    if (n == -1){
        return -1;
    }
    
    // Receive Message from arduino
    read(i2c_bus, buffer, buffSize);
    strtok(buffer, "\n");
  
    // Print Buffer
    printf("%s \n", buffer);

    // Return size of buffer
    return strlen(buffer);
}

int pollIMU(char buffer[], const int buffSize){
    // Attempt to connect to Magnometer
    if (ioctl(i2c_bus,I2C_SLAVE, MAGNO_I2C) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        return -1;
    }

    //
}

int openI2C(){
    // Open Connection to bus
    if ((i2c_bus = open("/dev/i2c-1", O_RDWR)) < 0) {
        printf("Failed to open the bus.");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        return -1;
    close(i2c_bus);
    }

    // Success
    return 1;
}

void sighandler(int signum){
    // Close everything
    close(i2c_bus);
    close(sockfd);
    close(clientfd);
    
    // Exit
    exit(-1);    
}
