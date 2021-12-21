#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

main() {

  // Server address object
  struct sockaddr_in serv_addr;

  // 2 Bytes
  short sequencecounter = 0;

  // 2 Bytes
  short * server_sequence;

  // Buffer to send
  char buff[1472];

  // Looping variable
  int i;

  // Arbitrary data
  for (i = 0; i < 1472; i++) {

    buff[i] = 'a';

  }

  // Blocks to send, starting at 1
  int N = 1;

  // Track the last throughput and current throughput
  double tp_last = 0;
  double throughput = 1;

  // Size of message
  int size;

  // Server socket
  int serv_sock;

  // Size of sockaddr_in object
  int serv_addr_length = sizeof(serv_addr);

  // Setup variables to track time 
  struct timeval current_time;
  struct timeval after_time;

  // Character array to receive acknowledgement message
  char acknowledge[10];

  // Setup server address
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(7494);
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Server socket
  serv_sock = socket(AF_INET, SOCK_DGRAM, 0);

  // Execute ping command on 127.0.0.1, store average into a variable
  FILE * fp;
  fp = popen("ping -c 4 127.0.0.1 | tail -1| awk '{print $4}' | cut -d '/' -f 2", "r");
  char file_buffer[10];
  fscanf(fp, "%s", file_buffer);
  pclose(fp);

  double ping;
  sscanf(file_buffer, "%lf", & ping);

  // While the change between throughput is not less than 5% (or exactly 0), keep going
  while ((fabs((tp_last - throughput) / throughput) > 0.05) || ((int) throughput == 0)) {

    // Sleep so that the output of the program is readable
    sleep(1);

    printf("SENDING FOR N = %d\n", N);
    gettimeofday( & current_time, NULL);

    // Reset sequence counter
    sequencecounter = 0;

    // Send the required number of blocks N
    for (i = 0; i < N; i++) {

      // Put sequence number into 2 byte character array
      buff[0] = sequencecounter & 0xff;
      buff[1] = (sequencecounter >> 8) & 0xff;

      sendto(serv_sock, buff, 1472, 0, (struct sockaddr * ) & serv_addr, serv_addr_length);
      sequencecounter++;

    }

    // Keep track of message that were actually received
    int received_size;
    int received_count = 0;

    // Take in everything in the message queue
    for (i = 0; i < N + (N / 10); i++) {
      received_size = recvfrom(serv_sock, acknowledge, 10, MSG_DONTWAIT, (struct sockaddr * ) & serv_addr, & serv_addr_length);

      if (received_size > 0) {
        received_count++;
      }
    }

    gettimeofday( & after_time, NULL);

    tp_last = throughput;

    // Calculate throughput
    throughput = (received_count * 1518 * 8) / (((after_time.tv_usec - current_time.tv_usec) / 1000.0) - ping);

    // Print out throughput
    printf("THROUGHPUT: %lf bits / second\n", throughput / 1000);

    // Check to see what change actually was
    printf("CHANGE: %lf\n", fabs((tp_last - throughput) / throughput));

    // Double N
    N *= 2;

  }

  // Terminate server
  char fin[1472] = "done";
  sendto(serv_sock, fin, 1472, 0, (struct sockaddr * ) & serv_addr, serv_addr_length);

}
