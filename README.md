# CPRE489

### Homework 2

Homework 2 focused on creating a basic throughput tester for a network. The client will keep sending more and more data to the server until the server can't handle it anymore.
The client then analyzes the number of received packets and uses a formula to deterimne the data throughput from the client to the server.

### Lab 2

Lab 2 involved setting up a UDP connection and using this connection to stream a video to a server. A source IP, source port, destination IP, destination port,
and loss rate are required as command line arguments to the client. Different loss rates will cause some packets to not be forwarded to the server / video stream.

### Lab 3

Lab 3 was largely setting up a file transfer protocol. The client and the server had to be set up. The client needed to establish a control connection that will be
used to send a directory listing from the server to the client. The client will then select a file on the server that it wants to download. The file will be sent over
a new data connection from the server to the client.

### Lab 5

Lab 5 required me to implement the Go-Back-N protocol. We were given a window that could be shifted in the utilities file. Using this window, we needed to register if
there was a bit error in one of the sent packets (received NAK from secondary) and mark it to be re-sent from primary. Ultimately, the user can type a message and it will
be completely received at secondary after all packages have been received correctly.

### Project

The project that I worked on was a Multipath TCP implementation. I mainly worked on the client for this project. 3 Different processes were created with fork after their respective connections were established. A control connection was also established. The data mapping would be sent on the control connection, and the actual data packets
would be sent on the 3 process connections. The data was sent in a cyclic manner (thread1, then thread2, then thread3, then back to thread1). The server was to receive the
mapping and use it to verify that the message was re-constructed correctly.
