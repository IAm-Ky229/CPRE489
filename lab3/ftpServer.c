#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char * argv[]) {

  // Server address object
  struct sockaddr_in server_addr;

  // Create new connection for new_port1
  struct sockaddr_in server_addr_new_port1;

  // Server address object for new_port2
  struct sockaddr_in server_addr_new_port2;

  // Client address object for new_port2
  struct sockaddr_in client_addr_new_port2;

  // IP of the server
  char server_IP[10] = "127.0.0.1";

  // Store the server's current socket
  int server_control_socket;

  // Store the accepted connection's socket
  int connection_control_socket;

  // Used to listen on new_port2
  int server_newport2_socket;

  // Used to accept connection to new_port2
  int file_transfer_socket;

  // Message buffer
  char buffer[1024];

  // Size of client address object
  int client_address_length = sizeof(client_addr_new_port2);

  // Port numbers
  int port_twenty = 7682;
  int port_twentyone = 6823;
  int new_port1 = 7494;
  int new_port2 = 8237;

  // File pointer
  FILE * fp;

  // Strings to store directory listing / file read buffer
  char directoryListing[500];
  char fileReadBuffer[75];

  // Allocate new server control socket
  server_control_socket = socket(AF_INET, SOCK_STREAM, 0);

  if ((server_control_socket) < 0) {
    printf("Could not open a control server socket\n");
    return -1;
  }

  // Setup main server port and IP
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_twentyone);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if ((bind(server_control_socket, (struct sockaddr * ) & server_addr, sizeof(server_addr))) < 0) {
    printf("Couldn't bind to the control server socket\n");
    return -1;
  }

  // Listen for connections on the main socket
  if (listen(server_control_socket, 10) != 0) {
    printf("Listen failed for control socket\n");
    return -1;
  }

  // Accept the connection we are listening for
  printf("Waiting to accept control connection\n");
  connection_control_socket = accept(server_control_socket, (struct sockaddr * ) & client_addr_new_port2, & client_address_length);

  if (connection_control_socket < 0) {
    printf("Accept control connection failed\n");
    return -1;
  }

  printf("Control connection accepted\n");

  // Read in a command from the client
  bzero(buffer, sizeof(buffer));
  printf("Waiting to read from control...\n");
  read(connection_control_socket, buffer, 1024);
  printf("Received from control: %s\n", buffer);

  // Read in the ls command server side
  fp = popen("ls", "r");
  while (fscanf(fp, "%s", fileReadBuffer) != EOF) {
    strcat(fileReadBuffer, "\n");
    strcat(directoryListing, fileReadBuffer);
  }
  pclose(fp);

  // Create new connection for new_port1
  server_addr_new_port1.sin_family = AF_INET;
  server_addr_new_port1.sin_port = htons(new_port1);
  server_addr_new_port1.sin_addr.s_addr = inet_addr(server_IP);

  // Create data transfer socket
  int server_newport1_socket;
  server_newport1_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (server_newport1_socket == -1) {
    printf("There was an error allocating the new_port1 socket\n");
    return -1;
  }

  if (connect(server_newport1_socket, (struct sockaddr * ) & server_addr_new_port1, sizeof(server_addr_new_port1)) != 0) {
    printf("Connection with server on new_port1 failed\n");
    return -1;
  }

  // Write to and close the data connection socket
  write(server_newport1_socket, directoryListing, strlen(directoryListing));
  close(server_newport1_socket);

  // Read in the file that is to be opened
  bzero(buffer, 1024);
  read(connection_control_socket, buffer, 1024);
  printf("Received from control: %s\n", buffer);

  char to_open[50];

  int j = 4;
  while (buffer[j] != ' ') {
    to_open[j - 4] = buffer[j];
    j++;
  }

  // Open the file the client requested
  printf("Opening file: %s\n", to_open);

  FILE * to_send = fopen(to_open, "r");
  if (to_send == NULL) {
    printf("That file doesn't exist.\n");
    return -1;
  }

  // Read all the data from the file
  char total_file_contents[8192];
  char file_data[200];

  while (fgets(file_data, 200, to_send)) {
    strcat(total_file_contents, file_data);
  }

  fclose(to_send);

  // Send data on the new_port2 connection
  server_addr_new_port2.sin_family = AF_INET;
  server_addr_new_port2.sin_port = htons(new_port2);
  server_addr_new_port2.sin_addr.s_addr = inet_addr(server_IP);

  server_newport2_socket = socket(AF_INET, SOCK_STREAM, 0);

  // Allocate new server socket
  if ((server_newport2_socket) < 0) {
    printf("Could not open a new_port2 socket\n");
    return -1;
  }

  if ((bind(server_newport2_socket, (struct sockaddr * ) & server_addr_new_port2, sizeof(server_addr_new_port2))) < 0) {
    printf("Couldn't bind to the new_port2 socket\n");
    return -1;
  }

  // Listen for connections on the data
  if (listen(server_newport2_socket, 10) != 0) {
    printf("Listen on new_port2 failed\n");
    return -1;
  }

  file_transfer_socket = accept(server_newport2_socket, (struct sockaddr * ) & client_addr_new_port2, & client_address_length);

  // Send file data
  write(file_transfer_socket, total_file_contents, 8192);

  // Close all connections
  close(connection_control_socket);
  close(server_control_socket);
  close(server_newport2_socket);
}

