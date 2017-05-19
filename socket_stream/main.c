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
#include <time.h>


///////////////////////////////////////////////////////////////////////
// Command Tags
#define SET_WHEEL_SPEED "SWV"		// Set the Wheel Speeds
#define GET_ODOMETRY "GOM"			// Get Odometry Measurement
#define GET_RANGE_READING "GRM"		// Get Range Measurement
#define GET_IMU_READING "GIR"		// Get IMU Measurement
#define SHUTDOWN_COMMS "SDC"		// Close Connection
///////////////////////////////////////////////////////////////////////

// Define Port to listen on
#define RPI_PORT 1618

// Set the max size of messages to be sent
#define SOCKET_MSG_SIZE 1026

// Set the Arduino Address
#define ARDUINO_I2C 0x05
#define I2C_MSG_SIZE 32

// Constants
#define PI 3.14159265359
#define MM_TO_M 0.001

// Define Wheel Parameters
#define WHEEL_SPEED_FACTOR 178
#define WHEEL_DIAMETER 0.0665
#define WHEEL_NSPOKE 8
#define WHEEl_TRACK (0.07366*2)
#define ENCODER_PER_PULSE (PI*WHEEL_DIAMETER)/(2*WHEEL_NSPOKE)

// Sonar Measurements calculations
#define NUM_SONAR 8
#define SONAR_TOF 0.000276485625
#define SONAR_MAX_RANGE 3

// Global variables
int i2c_bus = 0;        // File handle for i2c bus
int sockfd = 0;         // File handle for server socket
int clientfd = 0;       // File handle for client socket
 
// Declare Functions
void sighandler(int signum);


void shutdownComms();

int processMsg(char sendBuff[], const char* msg);

int pollArduino(char buffer[], const char msg[], const int buffSize);

int openI2C();

void str2double(double* num, const char str[], const int nNum);

int openSocket();

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	// Setup Signal Handler
	signal(SIGINT, sighandler);
		signal(SIGTERM, sighandler);
		signal(SIGHUP, sighandler);
		signal(SIGQUIT, sighandler);

	// Setup I2C Connection to Arduino
	if (openI2C() != 1) {return -1;}

	// Setup Socket Connection to Computer
	if (openSocket() !=  1) return -1;   

	// Start Connection Timer
	time_t startTime = time(NULL);

	// Create Buffer for reading and sending messages
	char sendBuff[SOCKET_MSG_SIZE];
	char recvBuff[SOCKET_MSG_SIZE];

	int isAlive = 1;    // Flag for client is still connected
	size_t n;             // Length of received/sent message
	int msgStatus = -1;     // Response from processMsg
	double respTime;        // Stores the time at which the msg is responded too (roughly), in seconds since start
	int errorCount = 0;
	while (isAlive==1){
		// Clear the buffers
		memset(sendBuff, 0, sizeof(sendBuff));
		memset(recvBuff, 0, sizeof(recvBuff));

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
			sprintf(sendBuff, "%.f ", respTime);

			// Process message
			msgStatus = processMsg(sendBuff, recvBuff);

			// Report Success
			n = write(clientfd, sendBuff, sizeof(sendBuff));
			if (n < 0) printf("ERROR: Unable to write to client");

			// Check for Exit Flag
			if(msgStatus < 0){
						  errorCount++;
						} else {
						  errorCount = 0;
						}
		}

		// Check for too many errors
		if(errorCount > 20) isAlive = 0;
	}

	// Close the connection
		shutdownComms();

	return 0;
}

int openSocket(){
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
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
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

	return 1;
}

// Process MSG
int processMsg(char sendBuff[], const char* msg) {
	// Report Message
	printf("\tMSG: %s\n", msg);

	// Message Response
	int startIndex = 8;
	char resp[SOCKET_MSG_SIZE];
	int status;

	if (strncmp(msg, SET_WHEEL_SPEED, 3) == 0) {
		// Set the wheel speeds
		char motorState[I2C_MSG_SIZE];
			
				// Convert To Motor Commands
				double speed [2];
				str2double(speed, msg+3, 2);

				// Convert to speed comands
				double speedCmd[2] = {0};
				for(int i = 0; i < 2; i++){
					speedCmd[i] = WHEEL_SPEED_FACTOR * speed[i] / (WHEEL_DIAMETER * PI); 
					
					// Check for Max Speed
					if (speedCmd[i] > 99) speedCmd[i] = 99;
				}

		// Create Message for Arduino
		char cmd[I2C_MSG_SIZE];
		sprintf(cmd, "W %.f %.f", speedCmd[0], speedCmd[1]);
	  
		// Send Command to Arduino
		pollArduino(motorState, cmd, I2C_MSG_SIZE);
		
		// ACK Motor Command
		strcat(sendBuff, motorState);
		strcat(sendBuff, " ");
		status = 1;
		
	} else if (strncmp(msg, GET_ODOMETRY, 3) == 0) {
		// Request Odmetry Readings from Arduino
		char odometryData[I2C_MSG_SIZE];
		printf("Requesting Odometry\n");
				int dataSize = pollArduino(odometryData, "O", I2C_MSG_SIZE);
		
		// Check Read was successful
		if(dataSize <= 0){
					printf("ERROR: No Odometry Data Returned\n");
					return -1;
				}

		// Convert to double
		double wheelTravel[2] = {0};
		str2double(wheelTravel, odometryData, 2);

		// Convert Clicks to Fwd Distance and Angular Rotation
		double FwdDist, AngTurn;
		FwdDist = ENCODER_PER_PULSE * (wheelTravel[0] + wheelTravel[1])/2;
		AngTurn = ENCODER_PER_PULSE * (wheelTravel[1] - wheelTravel[0])/ (PI*WHEEl_TRACK);

		// Report Odometry to Computer
		sprintf(sendBuff + strlen(sendBuff), "%.3f %.3f ", FwdDist, AngTurn);

		status = 1;

	} else if (strncmp(msg, GET_RANGE_READING, 3) == 0) {
		// Request Sonar Readings from Arduino
		char sonarData[I2C_MSG_SIZE];
		char cmd[I2C_MSG_SIZE];
		double sonar = 0;
				for(int i = 0; i < NUM_SONAR; i++){
			// Construct Command to get Sonar Reading
			sprintf(cmd, "s%d", i);

			// Poll Arduino
			int dataSize = pollArduino(sonarData, cmd, I2C_MSG_SIZE);

			// Compute Sonar Measurement
			sonar = strtod(sonarData, NULL)* SONAR_TOF;

			// Append to sendBuff
			if (sonar < SONAR_MAX_RANGE){
				sprintf(sendBuff + strlen(sendBuff), "%.3f ", sonar);
			} else{
				sprintf(sendBuff + strlen(sendBuff), "NaN ");
			}
		status = 1;

	} else if (strncmp(msg, GET_IMU_READING, 3) == 0) {
		// Report Failue
				sprintf(sendBuff, "ERROR IMU NOT CONNECTED");
		status = 1;

	} else if (strncmp(msg, SHUTDOWN_COMMS, 3) == 0) {
		// Shutdown comms
				sprintf(sendBuff, "Shutting down comms");
		status = -1;

	} else {
		// Echo message
		sprintf(sendBuff, "UNKNOWN COMMAND: [%s]", msg);
				status = 1;
	}

	// Check that response is not too long
	size_t respLen = strlen(resp);
	if (respLen > (SOCKET_MSG_SIZE - startIndex - 1)) {
		respLen = SOCKET_MSG_SIZE - startIndex - 1;
	}

	// Copy response into send buffer
	//strncpy(sendBuff+startIndex, resp, respLen);
		printf("%s\n", sendBuff);

	return status;
}

int pollArduino(char buffer[], const char msg[], const int buffSize){
	char buf[128];
	sprintf(buf, "\tSending to Arduino: %s \n", msg);
	printf(buf);

	// Send msg to arduino
	int n = write(i2c_bus, msg, strlen(msg));
	if (n == -1){
		return -1;
	}
	
	// Receive Message from arduino
	read(i2c_bus, buffer, buffSize);
	strtok(buffer, "\n");
  
	// Print Response
	sprintf(buf, "\tRecieved from Arduino: %s \n", buffer);
	printf(buf);

	// Return size of buffer
	return strlen(buffer);
}

int openI2C(){
	// Open Connection to bus
	if ((i2c_bus = open("/dev/i2c-1", O_RDWR)) < 0) {
		printf("Failed to open the bus.");
		/* ERROR HANDLING; you can check errno to see what went wrong */
		return -1;
	close(i2c_bus);
	}

	// Attempt to talk to Slave
	if (ioctl(i2c_bus,I2C_SLAVE, ARDUINO_I2C) < 0) {
		printf("Failed to acquire bus access and/or talk to slave.\n");
		/* ERROR HANDLING; you can check errno to see what went wrong */
		return -1;
	}

	// Success
	return 1;
}

void sighandler(int signum){
	// Close comms
	shutdownComms();	
	
	// Exit
	exit(-1);    
}

void str2double(double* num, const char str[], const int nNum){
// str2double: Extracts at most n double number from string

	// Pointer to start of unparsed str
	char* pEnd;
	
	// Get first number
	num[0] = strtod(str, &pEnd);
	
	// Get the rest of the numbers
	for(int i = 1; i < nNum; i++){
		num[i] = strtod(pEnd, &pEnd);
	}
}
void shutdownComms(){
  // Closes connections to everything
  close(i2c_bus);
  close(sockfd);
  close(clientfd);
}
