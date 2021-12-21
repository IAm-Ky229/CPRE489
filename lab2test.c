#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

main( int argc, char* argv[] ) {

struct sockaddr_in ServAddr;

  char buff[256] = "hello world";
  int n, fd, ServerAddrLen;
  ServerAddrLen  = sizeof(ServAddr);

 // Server port and IP
 ServAddr.sin_family = PF_INET;
 ServAddr.sin_port = htons(7494);
 ServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

 // Create socket
 fd = socket(PF_INET, SOCK_DGRAM, 0);

 if (fd < 0) {
   printf("Could not create a socket.");
   return -1;
 }

 printf("Socket created\n");


 // if((sendto(fd, buff, strlen(buff), 0, (struct sockaddr*) &ServAddr, ServerAddrLen)) < 0) {
 // printf("Can't send\n");
 // return -1;
 //}


 for(;;) {
 if( (recvfrom(fd, buff, 256, 0, (struct sockaddr*) &ServAddr, &ServerAddrLen) < 0)) {
       printf("Couldn't receive \n");
       return -1;
     }

   printf("Msg from server: %s\n", buff);
 }


 close(fd);


}

