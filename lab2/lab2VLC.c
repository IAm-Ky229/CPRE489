#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

main(int argc, char * argv[]) {

  // Socket objects
  struct sockaddr_in VLCSourceAddr;
  struct sockaddr_in VLCDestAddr;

  // Store command line arguments
  char * VLC_source_ip;
  int VLC_source_port;
  char * VLC_dest_ip;
  int VLC_dest_port;
  int loss_rate;

  // Other variables we are going to need
  int VLCSourceLength;
  char buff[4096];
  int n;
  int VLCServerSocket;
  int VLCDestSocket;

  // Actually assign command line arguments
  VLC_source_ip = argv[1];
  VLC_source_port = atoi(argv[2]);
  VLC_dest_ip = argv[3];
  VLC_dest_port = atoi(argv[4]);
  loss_rate = atoi(argv[5]);
  printf("%s, %d, %s, %d, %d", VLC_source_ip, VLC_source_port, VLC_dest_ip, VLC_dest_port, loss_rate);

  // Server port and IP
  VLCSourceAddr.sin_family = AF_INET;
  VLCSourceAddr.sin_port = htons(VLC_source_port);
  inet_aton(VLC_source_ip, & VLCSourceAddr.sin_addr);

  // Client port and IP
  VLCDestAddr.sin_family = AF_INET;
  VLCDestAddr.sin_port = htons(VLC_dest_port);
  inet_aton(VLC_dest_ip, & VLCDestAddr.sin_addr);

  // Create socket for server
  VLCServerSocket = socket(AF_INET, SOCK_DGRAM, 0);

  if ((bind(VLCServerSocket, (struct sockaddr * ) & VLCSourceAddr, sizeof(VLCSourceAddr))) < 0) {

    printf("Could not bind to the port.");
    return -1;

  }

  // Create socket for client
  VLCDestSocket = socket(AF_INET, SOCK_DGRAM, 0);

  printf("\n");
  printf("Binding finished\n");
  printf("Listening...\n");

  // Initialize random number generator
  time_t t;
  srand((unsigned) time( & t));
  int random_loss;

  for (;;) {

    random_loss = rand() % 100;

    // Recieve packets from VLC stream
    if ((n = recvfrom(VLCServerSocket, buff, sizeof(buff), 0, (struct sockaddr * ) & VLCSourceAddr, & VLCSourceLength)) < 0) {
      printf("Couldn't receive\n");
      return -1;
    }

    // Print to termainal
    printf("Msg: %s\n", buff);

    if (loss_rate < random_loss) {
      // Send packets to the destination VLC player
      sendto(VLCDestSocket, buff, n, 0, (struct sockaddr * ) & VLCDestAddr, VLCSourceLength);

    } else {
      printf("Packet lost\n");
    }

  }
}