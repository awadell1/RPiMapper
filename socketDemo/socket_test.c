#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

// Define Max Packet Size
#define MAX_PACKET_SIZE 1025

// Define Socket Port
#define SOCK_PORT 8888

void setWheelSpeed(float wheelLeft, float wheelRight){
  // Function to update the wheel speeds
  printf("\tSetting Wheel Speeds (%f, %f)\n", wheelLeft, wheelRight);
}

const char * readSonar(char *sonar){
  // Read Sonar Values
  sonar = "1,2,5,3";
  return sonar;
}

const char * parsePacket(const void *packet){
  // Copy packet into buffer
  char buff[MAX_PACKET_SIZE];
  const char dlm[] = ",";
  memcpy(buff, packet, strlen(packet)+1);

  // Get First Token
  char *cmd = strtok(buff, dlm);

  printf("Recieved Command: %s\n", cmd);

  if(strcmp(cmd, "wheelSpeed") == 0){
    // Extract Wheel Speeds
    float wheelLeft = atof(strtok(NULL, dlm));
    float wheelRight = atof(strtok(NULL, dlm));

    // Set Wheel Speeds
    setWheelSpeed(wheelLeft, wheelRight);

    // Report Success
    return "done";
  }
 //else if(strcmp(cmd, "readSonar") == 0){
 //  // Return Sonar Readings
 //  return readSonar();
 //}

}

int main(int argc, char *argv[])
{
  int sock = 0, clientSock = 0, fromlen = 0, bufLength = 0;
  struct sockaddr_in serv_addr;
  struct sockaddr_in from;
  char sendBuff[MAX_PACKET_SIZE+1];
  char recvBuff[MAX_PACKET_SIZE+1];
  time_t ticks;

  // Create a IPv4 socket for streaming data
  sock = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, '\0', sizeof(serv_addr));
  memset(sendBuff, '\0', sizeof(sendBuff));

  // Set the Sockets IP to any IPv4 address on port 5000
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(SOCK_PORT);

  // Apply the settings in serv_addr to the socket
  bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

  // Only want one person controlling the RPi at once
  listen(sock, 1);

  // Wait for connnection
  printf("Waiting for a connection \n");
  printf("Port: %d\n", SOCK_PORT);
  fromlen = sizeof(struct sockaddr_in);
  
  clientSock = accept(sock, (struct sockaddr*)NULL, NULL);

  printf("Client Connected\n");  

  // Do forever
  while(1)
  {
    // Clear Recieve Buffer
    bzero(&recvBuff, sizeof(recvBuff));

    // Receive Packet
    bufLength = read(clientSock, recvBuff, MAX_PACKET_SIZE);

    // Print Message
    recvBuff[bufLength] = 0;
    const char* resp = parsePacket(&recvBuff);
    
    // Respond to the client
    ticks = time(NULL);
    snprintf(sendBuff, sizeof(sendBuff), "%s\r\n", resp);
    write(clientSock, sendBuff, sizeof(sendBuff));
  
  }
}
