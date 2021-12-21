#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "ccitt16.h"
#include "utilities.h"
#include "introduceerror.h"

void primary(int sockfd, double ber) {

  // Read size from server
  int read_size;

  // Actual reply from server
  char srv_reply[150];

  // Used to read user input
  char msg[100];

  // Buffer for storing 13 packets to send
  struct packet_buffer packets[13];

  // Start with the beginning of the buffer
  int window[3] = {
    0,
    1,
    2
  };

  // Iterator variables
  int i;
  int j;

  // Variable to count the current place in sequence
  unsigned char sequence = 0;

  // Default sequence to resend is -1 (received all ACKs).
  int resend = -1;

  // Packet sending variable so the packet buffer is  not updated by error function
  unsigned char tosend[6];

  // Track how many packets we have ACKed so far
  int acked_PKTS = 0;

  // Track the total number of transmissions
  int total_transmissions = 0;

  while (1) {

    // abcdefghijklmnopqrstuvwxyz
    printf("Enter message : ");
    fgets(msg, 100, stdin);

    // Build all the packets
    for (i = 0; i < 13; i++) {
      unsigned char data[2] = {
        msg[i * 2],
        msg[i * 2 + 1]
      };
      buildPacket(packets[i].packet, DATA_PACKET, data, sequence);
      sequence++;
    }

    // For verification purposes, print all the packets
    for (i = 0; i < 13; i++) {
      printf("PACKET %d:\n", i);
      printPacket(packets[i].packet);
    }

    while (acked_PKTS < 13) {
      // Send everything in the window
      for (i = 0; i < 3; i++) {

        sleep(1);

        // Don't send anything outside the window
        if (window[i] < 13) {
          // This will introduce error before we send
          for (j = 0; j < 6; j++) {
            tosend[j] = packets[window[i]].packet[j];
          }
          IntroduceError(tosend, ber);

          if (send(sockfd, tosend, 6, 0) < 0) {
            perror("Send failed");
          }
          printf("SENT PACKET: %d\n", window[i]);

          if ((read_size = recv(sockfd, srv_reply, 149, 0)) < 0) {
            perror("recv failed");
          }

          // Print out server's reply
          printPacket(srv_reply);

          // If we received an ACK, check our place in the sequence
          if (srv_reply[0] == ACK_PACKET) {
            // If this is an ACK for the next packet in the sequence, increment the counter
            if (srv_reply[1] == (window[i] + 1)) {
              acked_PKTS++;
            }

          }

          // If we find a NAK packet, we need to start resending at this index
          if (srv_reply[0] == NAK_PACKET) {
            if (resend == -1) {
              resend = i;
            }
          }

          // Print out the currently ACKed packets for verification
          printf("CURRENTLY ACKED: %d\n", acked_PKTS);
	  total_transmissions++;

        }

      }

      // If no NAKs, just shift the window 3
      if (resend == -1) {
        printf("SHIFTING WINDOW NORMAL: 3\n");
        shiftWindow(window, 3, 3);
      }

      // Else we gotta use the first NAK to determine shift amount
      if (!(resend == -1)) {
        printf("SHIFTING WINDOW SPECIAL: %d\n", resend);
        shiftWindow(window, 3, resend);
      }

      // Assume all packets will be sent error-free in next iteration
      resend = -1;

      printf("--------------------------------\n");
    }

    printf("TOTAL RETRANSMISSIONS: %d\n", total_transmissions - 13);

  }

}

