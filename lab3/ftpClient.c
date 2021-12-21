#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char * argv[]) {

  // Used to bind to control socket
  struct sockaddr_in server_addr;

  // Server connection for new_port1;
  struct sockaddr_in server_addr_new_port1;

  // Server address object for newport 2
  struct sockaddr_in server_addr_new_port2;

  // Socket for new_port2
  int newport_2_socket;

  // Store the socket opened for a data connection
  int newport_1_socket;

  // Store the client's current control socket
  int client_control_socket;

  // Socket that will be used to request list of files
  int client_list_request_socket;

  // Size of newport_1 sockaddr_in object
  int server_addr_new_port1_size = sizeof(server_addr_new_port1);

  // Store the server's IP address
  char * server_IP;

  // Message buffer
  char buffer[1024];

  // Buffers to receive file data
  char file_read_buffer[8192];
  char file_read_line[200];

  // String to store file name we want to open
  char file_name[50];

  // Pointer to the file we want to create
  FILE * fp;

  // Define ports
  int port_twenty = 7682;
  int port_twentyone = 6823;
  int new_port1 = 7494;
  int new_port2 = 8237;

  // Check to see if we have the correct # of arguments
  if (argc < 2) {
    printf("Missing a server IP\n");
    return -1;
  }

  if (argc > 2) {
    printf("Too many arguments\n");
    return -1;
  }

  server_IP = argv[1];

  // Create server socket
  client_control_socket = socket(AF_INET, SOCK_STREAM, 0);

  if ((client_control_socket) == -1) {
    printf("There was an error allocating the main client socket\n");
    return -1;
  }

  // Setup server address and port
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_twentyone);
  server_addr.sin_addr.s_addr = inet_addr(server_IP);

  // Connect to server using IP given and port 21
  if (connect(client_control_socket, (struct sockaddr * ) & server_addr, sizeof(server_addr)) != 0) {
    printf("Connection with the server has failed\n");
    return -1;
  }

  printf("Connection with server control socket successful\n");

  client_list_request_socket = socket(AF_INET, SOCK_STREAM, 0);
  if ((client_list_request_socket) == -1) {
    printf("There was an error allocating the list request socket\n");
    return -1;
  }

  server_addr_new_port1.sin_family = AF_INET;
  server_addr_new_port1.sin_port = htons(new_port1);
  server_addr_new_port1.sin_addr.s_addr = inet_addr(server_IP);

  if (bind(client_list_request_socket, (struct sockaddr * ) & server_addr_new_port1, sizeof(server_addr_new_port1)) < 0) {
    printf("Could not bind the client list request socket\n");
  }

  // Format message buffer
  bzero(buffer, sizeof(buffer));
  sprintf(buffer, "LIST %d <CRLF>", new_port1);
  printf("SENDING: %s\n", buffer);

  // Send first command to server
  write(client_control_socket, buffer, strlen(buffer));

  // Listen for server reply
  printf("Listening for new server connection...\n");
  listen(client_list_request_socket, 5);

  printf("Waiting to accept...\n");
  newport_1_socket = accept(client_list_request_socket, (struct sockaddr * ) & server_addr_new_port1, & server_addr_new_port1_size);
  printf("Accepted server request to connect to new_port1\n");

  if (newport_1_socket == -1) {
    printf("Could not open socket for new_port1\n");
    return -1;
  }

  // Read from and close the data transfer socket
  read(newport_1_socket, buffer, 1024);
  close(newport_1_socket);

  // List server directory
  printf("Server directory listing:\n");
  printf("-------------------------\n");
  printf("%s", buffer);
  printf("-------------------------\n");

  // Select a file from the server's directory
  printf("Please select a file to download:\n");
  fgets(file_name, 50, stdin);
  file_name[strcspn(file_name, "\n")] = 0;

  bzero(buffer, 1024);
  sprintf(buffer, "RET %s %d <CRLF>", file_name, new_port2);

  write(client_control_socket, buffer, 1024);

  // Wait for the client to read the file
  sleep(1);

  // Create server socket for data transfer
  newport_2_socket = socket(AF_INET, SOCK_STREAM, 0);

  if ((newport_2_socket) == -1) {
    printf("There was an error allocating the new_port2 socket\n");
    return -1;
  }

  // Setup server address and port
  server_addr_new_port2.sin_family = AF_INET;
  server_addr_new_port2.sin_port = htons(new_port2);
  server_addr_new_port2.sin_addr.s_addr = inet_addr(server_IP);

  // Connect to server using IP given and port 20
  if (connect(newport_2_socket, (struct sockaddr * ) & server_addr_new_port2, sizeof(server_addr_new_port2)) != 0) {
    printf("Connection with server on new_port2 has failed\n");
    return -1;
  }

  // Receive the file data
  read(newport_2_socket, file_read_buffer, 8192);

  fp = fopen("received_file.txt", "w+");
  fputs(file_read_buffer, fp);

  fclose(fp);

  // Close literally everything
  close(client_control_socket);
  close(newport_2_socket);
}
