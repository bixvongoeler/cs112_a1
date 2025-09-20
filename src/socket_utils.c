#include "socket_utils.h"

void error(char *msg)
{
	perror(msg);
	exit(1);
}

int connect_to_client(int listen_sockfd)
{
	/* Create Client Socket Struct */
	struct sockaddr_in cli_addr;
	bzero((char *)&cli_addr, sizeof(cli_addr));

	/* Wait for connection */
	socklen_t clilen = sizeof(cli_addr);
	int client_sockfd =
		accept(listen_sockfd, (struct sockaddr *)&cli_addr, &clilen);
	if (client_sockfd < 0) {
		error("ERROR on accept");
	}

	return client_sockfd;
}

int connect_to_server(char *host, int port)
{
	int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sockfd < 0)
		error("ERROR opening socket");

	/* Create Server Host Struct */
	struct hostent *server;

	/* Get Server Info By Name */
	server = gethostbyname(host);
	if (server == NULL) {
		error("ERROR getting server by name");
	}

	/* Create Server Socket Struct */
	struct sockaddr_in serv_addr;
	bzero((char *)&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
	      server->h_length);
	serv_addr.sin_port = htons(port);

	/* Connect to server */
	if (connect(server_sockfd, (struct sockaddr *)&serv_addr,
		    sizeof(serv_addr)) < 0) {
		error("ERROR connecting to server");
	}

	return server_sockfd;
}

void forward_request_to_server(int server_sockfd, char *request)
{
	ssize_t bytes_written = write(server_sockfd, request, strlen(request));
	if (bytes_written < 0) {
		error("ERROR writing to server socket");
	}
	return;
}

void return_response_to_client(int client_sockfd, char *response,
			       size_t response_size)
{
	ssize_t bytes_written = write(client_sockfd, response, response_size);
	if (bytes_written < 0) {
		error("ERROR writing to client socket");
	}
	return;
}
