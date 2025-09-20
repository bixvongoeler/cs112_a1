#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <assert.h>
#include <netdb.h>

void error(char *msg);
int connect_to_client(int listen_sockfd);
int connect_to_server(char *host, int port);
void forward_request_to_server(int server_sockfd, char *request);
void return_response_to_client(int client_sockfd, char *response,
			       size_t response_size);

#endif
