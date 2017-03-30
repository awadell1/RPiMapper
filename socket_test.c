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

int main(int argc, char *argv[])
{
  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;

  char sendBuff[1025];

  time_t ticks;

  // Create a IPv4 socket for streaming data
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(sendBuff, '0', sizeof(sendBuff));

  // Set the Sockets IP to any IPv4 address on port 5000
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(5000);

  // Apply the settings in serv_addr to the socket
  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

  // Set the socket to only listen to at most 10 clients 
  listen(listenfd, 10);

  // Do forever
  while(1)
  {
    // Wait for a client to connect
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    // Respond to the client with the current time
    ticks = time(NULL);
    snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
    write(connfd, sendBuff, strlen(sendBuff));

    // Close the connection to the client
    close(connfd);

    // Sleep so CPU is not eaten up
    sleep(1);
  }
}
