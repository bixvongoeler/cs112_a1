/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	// File descriptors for the sockets
	int sockfd, newsockfd, portno;
	// Address size of the client
	socklen_t clilen;
	// Holds the received message
	char buffer[256];
	// Structure containing an internet address
	struct sockaddr_in serv_addr, cli_addr;
	// Number of bytes read/written
	int n;

	// Program Args
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	// Get the port number from the command line argument
	portno = atoi(argv[1]);

	/*
	 * Socket Creation
	 * (AF_INET denotes internet socket, SOCK_STREAM denotes TCP socket)
	*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// Returns -1 on failure
	if (sockfd < 0)
		error("ERROR opening socket");

	// Zero out the structure
	bzero((char *)&serv_addr, sizeof(serv_addr));

	// Get the port number from the command line argument
	portno = atoi(argv[1]);

	// Always set AF_INET as address family
	serv_addr.sin_family = AF_INET;
	// Convert port number to network byte order
	serv_addr.sin_port = htons(portno);
	// UBADDR_ANY = Ip address of the host, the machine running the server
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// Bind the socket to the address and port number
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		// On error, print error message and exit
		error("ERROR on binding");

	// Listen for incoming connections
	listen(sockfd, 5);

	// Block the process until a client connects
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
	if (newsockfd < 0)
		error("ERROR on accept");

	// Zero out the buffer
	bzero(buffer, 256);

	// Read the message from the client
	n = read(newsockfd, buffer, 255);
	if (n < 0)
		error("ERROR reading from socket");

	// Print the message received from the client
	printf("Here is the message: %s\n", buffer);

	// Write a response to the client
	n = write(newsockfd, "I got your message", 18);
	if (n < 0)
		error("ERROR writing to socket");

	// Close the client socket and the server socket
	close(newsockfd);
	close(sockfd);

	// return
	return 0;
}
