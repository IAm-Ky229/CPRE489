#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>

// Functions for 3 subflows
void thread1(int TCP_CONNECTION_1, int TCP_SOCKET_1, int CONTROL_SOCKET);
void thread2(int TCP_CONNECTION_2, int TCP_SOCKET_2, int CONTROL_SOCKET);
void thread3(int TCP_CONNECTION_3, int TCP_SOCKET_3, int CONTROL_SOCKET);

// Define ports we will use for connections here
#define CONTROL_PORT 2000
#define TCP_FLOW_PORT_1 2100
#define TCP_FLOW_PORT_2 2200
#define TCP_FLOW_PORT_3 2300

 // Global counter variable for threads
 int DATA_SEQUENCE_NUMBER = 0;

 // Pipes for data communication
 int pipethread[3][2];
 
 // Pipe for communicating current place in MPTCP data sequence
 int pipecounter[2];
 
 // Pipe for communicating what needs to be written to file 
 int pipefilewrite[2];

main() {
	
 // Used to store big string we are going to send
 char big_kahuna[992] = "";
 
 // Data communication pipe writing buffers
 char pipe_write1[10];
 char pipe_write2[10];
 char pipe_write3[10];
 
 // The file we are going to write to
 FILE *fp;

  // Socket and connection descriptors
 int control_socket, control_connection;
 int TCP_socket_1, TCP_connection_1;
 int TCP_socket_2, TCP_connection_2;
 int TCP_socket_3, TCP_connection_3;
 int TCP_1_length, TCP_2_length, TCP_3_length;

 // Server and client addresses
 struct sockaddr_in server_address, client_address;
 struct sockaddr_in TCP_server_1, TCP_client_1;
 struct sockaddr_in TCP_server_2, TCP_client_2;
 struct sockaddr_in TCP_server_3, TCP_client_3;
 
 // Track PID for each process so we can kill it at the end
 int thread1PID;
 int thread2PID;
 int thread3PID;

 // Buffer for sending / receiving
 char buffer[1024];
 
 // Create the file
 fp = fopen("client_mapping.txt", "w+");

 // Setup pipes for data
 int k;
 for(k = 0; k < 3; k++) {
   pipe(pipethread[k]);
 }
 
 // Setup counter and file write pipes
 pipe(pipecounter);
 pipe(pipefilewrite);

// Create control socket
control_socket = socket(AF_INET, SOCK_STREAM, 0);
if(control_socket == -1) {
  printf("Control socket creation error\n");
  exit(0);
}

bzero(&server_address, sizeof(server_address));

// Assign IP and port for server
server_address.sin_family = AF_INET;
server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
server_address.sin_port = htons(CONTROL_PORT);

if (connect(control_socket, (struct sockaddr*)&server_address, sizeof(server_address)) != 0) {
  printf("Control socket connection failed\n");
  exit(0);
}

// TCP flow 1 creation
TCP_socket_1 = socket(AF_INET, SOCK_STREAM, 0);
if(TCP_socket_1 == -1) {
  printf("TCP socket 1 creation error\n");
  exit(0);
}

// Assign IP and port for server
TCP_server_1.sin_family = AF_INET;
TCP_server_1.sin_addr.s_addr = inet_addr("127.0.0.1");
TCP_server_1.sin_port = htons(TCP_FLOW_PORT_1);

// Bind the socket
if((bind(TCP_socket_1, (struct sockaddr *)&TCP_server_1, sizeof(TCP_server_1))) < 0) {
  printf("TCP 1 bind error\n");
  exit(0);
}

// Listen on the socket
if (listen(TCP_socket_1, 5) != 0) {
  printf("TCP socket 1 listen error\n");
  exit(0);
}

TCP_1_length = sizeof(TCP_client_1);

printf("Waiting to accept tcp 1\n");

// Accept TCP connection 1
TCP_connection_1 = accept(TCP_socket_1, (struct sockaddr *)&TCP_client_1, &TCP_1_length);
if (TCP_connection_1 < 0 ) {
  printf("TCP connection 1 accept failed\n");
  exit(0);
}

 // Fork off the child process
 if((thread1PID = fork()) == 0) {
   thread1(TCP_connection_1, TCP_socket_1, control_socket);
 }
 
 
// TCP flow 2 creation
TCP_socket_2 = socket(AF_INET, SOCK_STREAM, 0);
if(TCP_socket_2 == -1) {
  printf("TCP socket 2 creation error\n");
  exit(0);
}

// Assign IP and port for server
TCP_server_2.sin_family = AF_INET;
TCP_server_2.sin_addr.s_addr = inet_addr("127.0.0.1");
TCP_server_2.sin_port = htons(TCP_FLOW_PORT_2);

// Bind the socket
if((bind(TCP_socket_2, (struct sockaddr *)&TCP_server_2, sizeof(TCP_server_2))) < 0) {
  printf("TCP 2 bind error\n");
  exit(0);
  }

// Listen on the socket
if (listen(TCP_socket_2, 5) != 0) {
  printf("TCP socket 2 listen error\n");
  exit(0);
}

TCP_2_length = sizeof(TCP_client_2);

printf("Waiting to accept tcp 2\n");

// Accept TCP connection 2
TCP_connection_2 = accept(TCP_socket_2, (struct sockaddr *)&TCP_client_2, &TCP_2_length);
if (TCP_connection_2 < 0 ) {
  printf("TCP connection 2 accept failed\n");
  exit(0);
}

 // Fork off the child process
 if((thread2PID = fork()) == 0) {
   thread2(TCP_connection_2, TCP_socket_2, control_socket);
 }
 

// TCP flow 3 creation
TCP_socket_3 = socket(AF_INET, SOCK_STREAM, 0);
if(TCP_socket_3 == -1) {
  printf("TCP socket 3 creation error\n");
  exit(0);
}

// Assign IP and port for server
TCP_server_3.sin_family = AF_INET;
TCP_server_3.sin_addr.s_addr = inet_addr("127.0.0.1");
TCP_server_3.sin_port = htons(TCP_FLOW_PORT_3);

// Bind the socket
if((bind(TCP_socket_3, (struct sockaddr *)&TCP_server_3, sizeof(TCP_server_3))) < 0) {
  printf("TCP 3 bind error\n");
  exit(0);
}

// Listen on the socket
if (listen(TCP_socket_3, 5) != 0) {
  printf("TCP socket 3 listen error\n");
  exit(0);
}

TCP_3_length = sizeof(TCP_client_3);

printf("Waiting to accept tcp 3\n");

// Accept TCP connection 3
TCP_connection_3 = accept(TCP_socket_3, (struct sockaddr *)&TCP_client_3, &TCP_3_length);
if (TCP_connection_3 < 0 ) {
  printf("TCP connection 3 accept failed\n");
  exit(0);
}

 // Fork off the child process
 if((thread3PID = fork()) == 0) {
   thread3(TCP_connection_3, TCP_socket_3, control_socket);
 }
 
bzero(buffer, 1024);

 // Assemble the block of data we're going to send
 char digits[10] = "0123456789";
 char lowercase[26] = "abcdefghijklmnopqrstuvwxyz";
 char uppercase[26] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
 int i;

 for(i = 0; i < 16; i++) {
   strcat(big_kahuna, digits);
   strcat(big_kahuna, lowercase);
   strcat(big_kahuna, uppercase);
}

char mainFileWrite[50] = "";

// Write to pipes initially so we don't block
write(pipecounter[1], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));
write(pipefilewrite[1], mainFileWrite, sizeof(mainFileWrite));


 for (i = 0; i < 992; i+= 12) {
	 
   // Read the counter update from thread 3
   read(pipecounter[0], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));
   
   // Write the data sequence number to thread 1
   write(pipethread[0][1], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));
   
   // Read from the file write pipe 
   read(pipefilewrite[0], mainFileWrite, sizeof(mainFileWrite));
   
   // Write to the file
   fprintf(fp, mainFileWrite);
   fprintf(fp, "\n");
   
   if(i + 3 < 992) {
   // Send on thread 1
   pipe_write1[0] = big_kahuna[i];
   pipe_write1[1] = big_kahuna[i+1];
   pipe_write1[2] = big_kahuna[i+2];
   pipe_write1[3] = big_kahuna[i+3];
   write(pipethread[0][1], pipe_write1, sizeof(pipe_write1));
   usleep(2500);
   }
   
	  // Read the counter update from thread 1
      read(pipecounter[0], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));
	  
	  // Write the counter update to thread 2
	  write(pipethread[1][1], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));
	  
	  // Read the sequence numbers we need to write to the file
	  read(pipefilewrite[0], mainFileWrite, sizeof(mainFileWrite));
	  
	  // Write to the file
      fprintf(fp, mainFileWrite);
	  fprintf(fp, "\n");
	  

   if(i + 7 < 992) {
   // Send on thread 2
   pipe_write2[0] = big_kahuna[i+4];
   pipe_write2[1] = big_kahuna[i+5];
   pipe_write2[2] = big_kahuna[i+6];
   pipe_write2[3] = big_kahuna[i+7];
   write(pipethread[1][1], pipe_write2, sizeof(pipe_write2));
   usleep(2500);
}

   // Read the counter update from thread 2
   read(pipecounter[0], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));
   
   // Write the counter update to thread 3
   write(pipethread[2][1], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));
   
   // Read the sequence numbers to write to the file
   read(pipefilewrite[0], mainFileWrite, sizeof(mainFileWrite));
   
   // Write to the file
   fprintf(fp, mainFileWrite);
   fprintf(fp, "\n");


   if(i + 11 < 992) {
     // Send on thread 3
     pipe_write3[0] = big_kahuna[i+8];
     pipe_write3[1] = big_kahuna[i+9];
     pipe_write3[2] = big_kahuna[i+10];
     pipe_write3[3] = big_kahuna[i+11];
     write(pipethread[2][1], pipe_write3, sizeof(pipe_write3));
	 usleep(2500);
   }
  fflush(stdout); 
 }

 // Terminate the threads
 kill(thread1PID, SIGKILL);
 kill(thread2PID, SIGKILL);
 kill(thread3PID, SIGKILL);

 // Close all active connections
 close(control_socket);
 close(TCP_socket_1);
 close(TCP_socket_2);
 close(TCP_socket_3);
 
 // Close the file
 fclose(fp);

 exit(0);

}

void thread1(int TCP_CONNECTION_1, int TCP_SOCKET_1, int CONTROL_SOCKET) {

 short buffer[8];
 char pipe_read[10];
 int thread1seq = 0;

 while(1) {
	 
	// Read the data sequence number update from thread 3
	read(pipethread[0][0], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));

   // Read 4 bytes from pipe 1
   read(pipethread[0][0], pipe_read, sizeof(pipe_read));
   printf("Thread 1 got: %s\n", pipe_read);

   bzero(buffer, sizeof(buffer));

   // Setup DSS and subflow sequence number mappings
   buffer[0] = DATA_SEQUENCE_NUMBER;
   buffer[1] = thread1seq;
   DATA_SEQUENCE_NUMBER++;
   thread1seq++;
   
   buffer[2] = DATA_SEQUENCE_NUMBER;
   buffer[3] = thread1seq;
   DATA_SEQUENCE_NUMBER++;
   thread1seq++;
   
   buffer[4] = DATA_SEQUENCE_NUMBER;
   buffer[5] = thread1seq;
   DATA_SEQUENCE_NUMBER++;
   thread1seq++;
   
   buffer[6] = DATA_SEQUENCE_NUMBER;
   buffer[7] = thread1seq;
   DATA_SEQUENCE_NUMBER++;
   thread1seq++;
   
   // Convert the sequence numbers to a string   
   int j;
   char fileWrite[50] = "";
   char convert[10] = "";
   for (j = 0; j < 8; j++) {
	bzero(convert, sizeof(convert));
	sprintf(convert, "%d, ", buffer[j]);
	strcat(fileWrite, convert);
   }
	  
   // Write the sequence numbers to the control socket
   write(CONTROL_SOCKET, buffer, sizeof(buffer));
   
   // Send 4 bytes of data
   write(TCP_CONNECTION_1, pipe_read, 4);
  
  // Update thread 2's sequence number
   write(pipecounter[1], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));
   
   // Pipe sequence numbers back to parent for file write
   write(pipefilewrite[1], fileWrite, sizeof(fileWrite));
   
   fflush(stdout);
 }
}

void thread2(int TCP_CONNECTION_2, int TCP_SOCKET_2, int CONTROL_SOCKET) {

 short buffer[8];
 char pipe_read[10];
 int thread2seq = 0;

 while(1) {
	 
   // Read the sequence number update from thread 1
   read(pipethread[1][0], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));

   // Read 4 bytes from pipe 2
   read(pipethread[1][0], pipe_read, sizeof(pipe_read));
   printf("Thread 2 got: %s\n", pipe_read);
   
   bzero(buffer, sizeof(buffer));

   // Setup DSS and subflow sequence number mappings
   buffer[0] = DATA_SEQUENCE_NUMBER;
   buffer[1] = thread2seq;
   DATA_SEQUENCE_NUMBER++;
   thread2seq++;
   
   buffer[2] = DATA_SEQUENCE_NUMBER;
   buffer[3] = thread2seq;
   DATA_SEQUENCE_NUMBER++;
   thread2seq++;
   
   buffer[4] = DATA_SEQUENCE_NUMBER;
   buffer[5] = thread2seq;
   DATA_SEQUENCE_NUMBER++;
   thread2seq++;
   
   buffer[6] = DATA_SEQUENCE_NUMBER;
   buffer[7] = thread2seq;
   DATA_SEQUENCE_NUMBER++;
   thread2seq++;
   
   // Convert the sequence number to a string
   int j;
   char fileWrite[50] = "";
   char convert[10] = "";
   for(j = 0; j < 8; j++) {
    bzero(convert, sizeof(convert));
	sprintf(convert, "%d, ", buffer[j]);
	strcat(fileWrite, convert); 
   }
	  
   // Write DSS mapping to the control socket
   write(CONTROL_SOCKET, buffer, sizeof(buffer));
   
   // Send 4 bytes of data
   write(TCP_CONNECTION_2, pipe_read, 4);
   
   // Update sequence number for thread 3
   write(pipecounter[1], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));
   
   // Pipe data back to parent for file write
   write(pipefilewrite[1], fileWrite, sizeof(fileWrite));
   
   fflush(stdout);
 }

}

void thread3(int TCP_CONNECTION_3, int TCP_SOCKET_3, int CONTROL_SOCKET) {

    short buffer[8];
    char pipe_read[10];
    int thread3seq = 0;
	
 while(1) {
	 
	 // Read the sequence number update from thread 2
	 read(pipethread[2][0], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));

   // Read from pipe 3
   read(pipethread[2][0], pipe_read, sizeof(pipe_read));
   printf("Thread 3 got: %s\n", pipe_read);
   
   bzero(buffer, sizeof(buffer));

   // Setup DSS and subflow sequence number mappings
   buffer[0] = DATA_SEQUENCE_NUMBER;
   buffer[1] = thread3seq;
   DATA_SEQUENCE_NUMBER++;
   thread3seq++;
   
   buffer[2] = DATA_SEQUENCE_NUMBER;
   buffer[3] = thread3seq;
   DATA_SEQUENCE_NUMBER++;
   thread3seq++;
   
   buffer[4] = DATA_SEQUENCE_NUMBER;
   buffer[5] = thread3seq;
   DATA_SEQUENCE_NUMBER++;
   thread3seq++;
   
   buffer[6] = DATA_SEQUENCE_NUMBER;
   buffer[7] = thread3seq;
   DATA_SEQUENCE_NUMBER++;
   thread3seq++;
   
   // Convert sequence numbers to string
   int j;
   char fileWrite[50] = "";
   char convert[10] = "";
   for (j = 0; j < 8; j++) {
	bzero(convert, sizeof(convert));
	sprintf(convert, "%d, ", buffer[j]);
	strcat(fileWrite, convert);
   }

   // Write DSS sequence numbers to control socket
   write(CONTROL_SOCKET, buffer, sizeof(buffer));
   
   // Send 4 bytes of data
   write(TCP_CONNECTION_3, pipe_read, 4);
   
   // Update thread 1's DSS number
   write(pipecounter[1], &DATA_SEQUENCE_NUMBER, sizeof(DATA_SEQUENCE_NUMBER));
   
   // Pipe back to parent for file write
   write(pipefilewrite[1], fileWrite, sizeof(fileWrite));
   
   fflush(stdout);
 }
}
