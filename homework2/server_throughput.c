#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>

main() {

  // Client and server addresses
  struct sockaddr_in serv_addr, client_addr;

  // Buffer to receive messages
  char buff[1472];

  // The socket to be allocated to the server
  int serv_sock;

  // Length of client address object
  int client_addr_length = sizeof(serv_addr);

  // Used to echo back a response
  char echo[10];

  // Used to track the client's position in sending blocks
  short client_sequence_counter = 0;

  // Number of blocks the server has received
  int servsequence = 0;

  // Acknowledge message to send back to client
  char acknowledge[10] = "ok";

  // Server address assignment
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(7494);
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Server socket and bind
  serv_sock = socket(AF_INET, SOCK_DGRAM, 0);
  bind(serv_sock, (struct sockaddr * ) & serv_addr, sizeof(serv_addr));

  while (1) {

    // First, receive message
    printf("BLOCKING\n");
    recvfrom(serv_sock, buff, 1472, 0, (struct sockaddr * ) & client_addr, & client_addr_length);

    // If the client is done, server is also done
    if(strcmp(buff, "done") == 0) {
      break;
    }

    // See where client is in sequence
    char b = buff[0];
    char a = buff[1];

    client_sequence_counter = (((short) a) << 8) | (0x00ff & b);
    printf("CLIENT SEQUENCE: %d\n", client_sequence_counter);

    // Acknowledge
    sendto(serv_sock, acknowledge, 10, 0, (struct sockaddr * ) & client_addr, client_addr_length);

    // Only echo server position in sequence if we have sent back 10 blocks
    if (servsequence % 10 == 0) {
      sprintf(echo, "%d", servsequence);
      sendto(serv_sock, echo, 10, 0, (struct sockaddr * ) & client_addr, client_addr_length);

    }

    // Increment server position in sequence
    servsequence++;
  }

}
