#include "proxy.h"

struct Proxy {
	int port;
	int listen_sockfd;
	struct sockaddr_in proxy_addr;
};

void handle_client_request(Proxy proxy, int client_sockfd);

Proxy proxy_new(int port)
{
	Proxy proxy;
	proxy = malloc((long)sizeof *(proxy));
	assert(proxy != NULL);

	/* Initialize All Values */
	proxy->port = port;

	/* Open Server Socket */

	/* Create Internet Socket Using TCP */
	proxy->listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (proxy->listen_sockfd < 0) {
		error("ERROR opening socket");
	}

	/* Enable Socket Reuse */
	int reuse = 1;
	int result = setsockopt(proxy->listen_sockfd, SOL_SOCKET, SO_REUSEADDR,
				(void *)&reuse, sizeof(reuse));
	if (result < 0) {
		perror("ERROR SO_REUSEADDR:");
	}

	/* Create Structs to hold Client and Server IPs */
	bzero((char *)&proxy->proxy_addr, sizeof(proxy->proxy_addr));

	/* Init Server Address Details */
	proxy->proxy_addr.sin_family = AF_INET;
	proxy->proxy_addr.sin_port = htons(proxy->port);
	proxy->proxy_addr.sin_addr.s_addr = INADDR_ANY;

	/* Bind Socket to Address */
	if (bind(proxy->listen_sockfd, (struct sockaddr *)&proxy->proxy_addr,
		 sizeof(proxy->proxy_addr)) < 0) {
		error("ERROR on binding");
	}

	/* Start Listening for incoming connections */
	listen(proxy->listen_sockfd, 5);

	/* Return New Proxy */
	return proxy;
}

void proxy_free(Proxy *proxy_ptr)
{
	assert(proxy_ptr != NULL);
	assert(*proxy_ptr != NULL);

	Proxy proxy = *proxy_ptr;

	/* Clean Up Internals */
	close(proxy->listen_sockfd);

	/* Free Struct */
	free(proxy);

	/* Nullify Pointer */
	*proxy_ptr = NULL;
}

void proxy_run(Proxy proxy)
{
	assert(proxy != NULL);

	/* Run the proxy server */
	printf("Running proxy server on port %d\n", proxy->port);

	while (1) {
		/* Accept Client Connection */
		int client_sockfd = connect_to_client(proxy->listen_sockfd);

		/* Handle client request */
		handle_client_request(proxy, client_sockfd);

		/* Close client socket */
		close(client_sockfd);
	}

	return;
}

void handle_client_request(Proxy proxy, int client_sockfd)
{
	assert(proxy != NULL);

	/* Read Full HTTP Request From Client */
	size_t restrict_size;
	char *request = read_http_request(client_sockfd, &restrict_size);

	printf("Request:\n%s\n", request);

	/* Parse HTTP Request */
	HTTPRequest parsed = parse_http_request(request);

	/* Check Cache (NOT YET IMPLIMENTED) */
	int cached = 0;

	if (cached) {
		printf("Cached Request\n");
	} else {
		/* Forward to Server */
		int server_sockfd = connect_to_server(parsed.host, parsed.port);

		/* Send Request to Server */
		forward_request_to_server(server_sockfd, request);

		/* Read Response from Server */
		size_t response_size;
		char *response = read_http_response(server_sockfd, &response_size);

		/* Print Response */
		// printf("%s", response);

		/* Return Response to Client */
		return_response_to_client(client_sockfd, response, response_size);

		/* Update Cache */
		printf("Updating Cache!\n");

		/* Close Connection with Server */
		close(server_sockfd);

		free(response);
	}

	free(request);
}
