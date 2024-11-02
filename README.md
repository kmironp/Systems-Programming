# Systems-Programming
 Creation of a C program under Linux that creates a 1-bit color depth (two-color) .bmp file that represents a graph illustrating the change of a randomly varying quantity over time. The program can operate in two modes: one generates the data (it is the sender), the other represents them (it is the receiver). Two copies of the program started in different modes (so it is a single program, but two processes) implement the data transfer through inter-process communication, either via a file or a socket.
