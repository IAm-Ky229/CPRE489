#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>

void thread1(int TCP_SOCKET_1);
void thread2(int TCP_SOCKET_2);
void thread3(int TCP_SOCKET_3);

#define CONTROL_PORT 2000
#define TCP_FLOW_PORT_1 2100
#define TCP_FLOW_PORT_2 2200
#define TCP_FLOW_PORT_3 2300

// Global counter variable for threads
int DATA_SEQUENCE_NUMBER = 0;

int flags;

// Global pipe variables
 int fd1[2];
 int fd2[2];
 int fd3[2];
 int pipestatus;
 char pipe_read[10];
 char* clientIP = "127.0.0.1";

main() {

  // Socket and connection descriptors
 int control_socket, control_connection, length;
 int TCP_socket_1, TCP_connection_1, TCP_1_length;
 int TCP_socket_2, TCP_connection_2, TCP_2_length;
 int TCP_socket_3, TCP_connection_3, TCP_3_length;

 // Client and server addresses
 struct sockaddr_in server_address, client_address;
 struct sockaddr_in TCP_server_1, TCP_client_1;
 struct sockaddr_in TCP_server_2, TCP_client_2;
 struct sockaddr_in TCP_server_3, TCP_client_3;

 // Buffer for sending / receiving messages
 char buffer1[4], buffer3[4], buffer2[4];
 short DSS[8];
 char message[1024] = "";

 // Setup pipe
 pipestatus = pipe(fd1);
 pipe(fd2);
 pipe(fd3);
 if (pipestatus == -1) {
   printf("Couldn't create the pipe\n");
   exit(0);
 }
 
	fcntl(fd1[0], F_SETFL, O_NONBLOCK);
	
	fcntl(fd2[0], F_SETFL, O_NONBLOCK);
	
	fcntl(fd3[0], F_SETFL, O_NONBLOCK);

// Create control socket
control_socket = socket(AF_INET, SOCK_STREAM, 0);
if(control_socket == -1) {
  printf("Control socket creation error\n");
  exit(0);
}

// Assign IP and port for server
server_address.sin_family = AF_INET;
server_address.sin_addr.s_addr = inet_addr(clientIP);
server_address.sin_port = htons(CONTROL_PORT);

// Bind the socket
if((bind(control_socket, (struct sockaddr *)&server_address, sizeof(server_address))) < 0) {
  printf("Connection socket bind failed %d\n", errno);
  exit(0);
}

// Listen on the socket
if (listen(control_socket, 5) != 0) {
  printf("Control socket listen error\n");
  exit(0);
}

length = sizeof(client_address);

printf("Waiting to accept control connection\n");

// Accept control connection
control_connection = accept(control_socket, (struct sockaddr *)&client_address, &length);
if (control_connection < 0 ) {
  printf("Control connection accept failed\n");
  exit(0);
}

// Create TCP socket 1
TCP_socket_1 = socket(AF_INET, SOCK_STREAM, 0);
if(TCP_socket_1 == -1) {
  printf("TCP socket 1 creation error\n");
  exit(0);
}
usleep(5000);

// Assign IP and port for server
TCP_server_1.sin_family = AF_INET;
TCP_server_1.sin_addr.s_addr = inet_addr(clientIP);
TCP_server_1.sin_port = htons(TCP_FLOW_PORT_1);

// Connect to client TCP 1
if (connect(TCP_socket_1, (struct sockaddr*)&TCP_server_1, sizeof(TCP_server_1)) != 0) {
  printf("TCP socket 1 connection failed\n");
  exit(0);
}

// Create TCP socket 2
TCP_socket_2 = socket(AF_INET, SOCK_STREAM, 0);
if(TCP_socket_2 == -1) {
  printf("TCP socket 2 creation error\n");
  exit(0);
}
usleep(5000);

// Assign IP and port for server
TCP_server_2.sin_family = AF_INET;
TCP_server_2.sin_addr.s_addr = inet_addr(clientIP);
TCP_server_2.sin_port = htons(TCP_FLOW_PORT_2);

// Connect to client TCP 2
if (connect(TCP_socket_2, (struct sockaddr*)&TCP_server_2, sizeof(TCP_server_2)) != 0) {
  printf("TCP socket 2 connection failed\n");
  exit(0);
}

// Create TCP socket 3
TCP_socket_3 = socket(AF_INET, SOCK_STREAM, 0);
if(TCP_socket_3 == -1) {
  printf("TCP socket 3 creation error\n");
  exit(0);
}
usleep(5000);

// Assign IP and port for server
TCP_server_3.sin_family = AF_INET;
TCP_server_3.sin_addr.s_addr = inet_addr(clientIP);
TCP_server_3.sin_port = htons(TCP_FLOW_PORT_3);

// Connect to client TCP 3
if (connect(TCP_socket_3, (struct sockaddr*)&TCP_server_3, sizeof(TCP_server_3)) != 0) {
  printf("TCP socket 3 connection failed\n");
  exit(0);
}

int subflow, n, i;
pid_t pids[3];
for(subflow = 0; subflow < 3; subflow++) {
  char databuff1[4];
  char databuff2[4];
  char databuff3[4];
  pids[subflow] = fork();
  if(pids[subflow] < 0) {
    perror("fork failed");
    exit(1);
  }
  if(pids[subflow] == 0) { //Child subflow
    switch(subflow){
      case 0:
        //Read on TCP_FLOW_PORT1
        while(n = read(TCP_socket_1, databuff1, 4)){
          //Pipe to parent process
          //close(fd1[0]);
          write(fd1[1], databuff1, sizeof(databuff1));
          printf("data subflow 1: %s\n", databuff1);
        }
        exit(0);
      case 1:
        //Read on TCP_FLOW_PORT2
        while(n = read(TCP_socket_2, databuff2, 4)){
          //Pipe to parent process
          //close(fd2[0]);
          write(fd2[1], databuff2, sizeof(databuff2));
          printf("data subflow 2: %s\n", databuff2);
        }
        exit(0);
      case 2:
        //Read on TCP_FLOW_PORT3
        while(n = read(TCP_socket_3, databuff3, 4)){
          //Pipe to parent process
          //close(fd3[0]);
          write(fd3[1], databuff3, sizeof(databuff3));
          printf("data subflow 3: %s\n", databuff3);
        }
        exit(0);
    }
  } else { //Parent subflow3
    //Read the pipe from the child process to get the message
  }
}

sleep(1);

//close(fd1[1]);
//close(fd2[1]);
//close(fd3[1]);
FILE *f = fopen("DSSmapping.txt", "w");

while(n = read(control_connection, DSS, sizeof(DSS))) {
  int j;
  //Read from subflow 1 pipe
  read(fd1[0], buffer1, 4);
  //for(i = 0; i < 4; i++) {
    //message[i] = buffer1[i];
  //}
  buffer1[4] = '\0';
  strcat(message, buffer1);
  printf("PIPE 1 RECONSTRUCTION: %s\n", buffer1);
  
  for(j = 0; j < 8; j++) {
    fprintf(f, "%d, ", DSS[j]);
  }
  fprintf(f, "\n");
  //Read from subflow 2 pipe
  bzero(DSS, sizeof(DSS));
  read(control_connection, DSS, sizeof(DSS));
  read(fd2[0], buffer2, 4);
  //for(i = 0; i < 4; i++) {
    //message[i] = buffer2[i];
  //}
  buffer2[4] = '\0';
  strcat(message, buffer2);
  
  printf("PIPE 2 RECONSTRUCTION: %s\n", buffer2);
  for(j = 0; j < 8; j++) {
    fprintf(f, "%d, ", DSS[j]);
  }
  fprintf(f, "\n");
  //Read from subflow 3 pipe
  bzero(DSS, sizeof(DSS));
  read(control_connection, DSS, sizeof(DSS));
  read(fd3[0], buffer3, 4);
  //for(i = 0; i < 4; i++) {
    //message[i] = buffer3[i];
  //}
  buffer3[4] = '\0';
  strcat(message, buffer3);
  
  printf("PIPE 3 RECONSTRUCTION: %s\n", buffer3);
  
  for(j = 0; j < 8; j++) {
    fprintf(f, "%d, ", DSS[j]);
  }
  fprintf(f, "\n");
  //Map the subflow sequence number to data sequence number
  usleep(5000);
  
  bzero(DSS, sizeof(DSS));
  bzero(buffer1, sizeof(buffer1));
  bzero(buffer2, sizeof(buffer2));
  bzero(buffer3, sizeof(buffer3));
}
fclose(f);

printf("Message %s\n", message);

 // Close all active connections
close(control_socket);
close(TCP_socket_1);
close(TCP_socket_2);
close(TCP_socket_3);
fflush(stdout);

}

