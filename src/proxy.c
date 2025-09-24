#include "proxy.h"

#define CACHE_SIZE 10

struct Proxy {
	int port;
	int listen_sockfd;
	struct sockaddr_in proxy_addr;
	Cache *cache;
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

	/* Create the Cache */
	proxy->cache = cache_create(CACHE_SIZE);
	if (proxy->cache == NULL) {
		close(proxy->listen_sockfd);
		free(proxy);
		error("ERROR creating cache");
	}

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
	cache_destroy(proxy->cache);

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

	/* Check Cache */
	int cache_result = cache_get(proxy->cache, parsed.full_url);

	if (cache_result >= 0) {
		/* Cache hit - data is fresh */
		printf("Cache HIT (fresh): %s\n", parsed.full_url);

		size_t cached_content_size;
		void *cached_content = cache_get_content(proxy->cache, cache_result,
							 &cached_content_size);

		if (cached_content) {
			/* Inject Age header */
			uint64_t age_seconds =
				cache_get_age_seconds(proxy->cache, cache_result);
			size_t new_response_size;
			char *response_with_age = inject_age_header(
				(char *)cached_content, cached_content_size,
				age_seconds, &new_response_size);

			if (response_with_age) {
				return_response_to_client(client_sockfd,
							  response_with_age,
							  new_response_size);
				free(response_with_age);
			} else {
				return_response_to_client(client_sockfd,
							  (char *)cached_content,
							  cached_content_size);
			}
			free(request);
			return;
		}
	} else if (cache_result == -2) {
		/* Cache hit but stale - need to refetch */
		printf("Cache HIT (stale): %s\n", parsed.full_url);
	} else {
		/* Cache miss */
		printf("Cache MISS: %s\n", parsed.full_url);
	}

	/* Forward to Server (cache miss or stale) */
	int server_sockfd = connect_to_server(parsed.host, parsed.port);

	/* Send Request to Server */
	forward_request_to_server(server_sockfd, request);

	/* Read Response from Server */
	size_t response_size;
	char *response = read_http_response(server_sockfd, &response_size);

	/* Parse Cache-Control for max-age */
	int max_age = parse_cache_control_max_age(response);
	printf("Server response max-age: %d seconds\n", max_age);

	/* Return Response to Client */
	return_response_to_client(client_sockfd, response, response_size);

	/* Update Cache */
	int cache_index = cache_put(proxy->cache, parsed.full_url, max_age);
	if (cache_index >= 0) {
		cache_store_content(proxy->cache, cache_index, response,
				    response_size);
		printf("Cached response for: %s\n", parsed.full_url);
	} else {
		printf("Failed to cache response for: %s\n", parsed.full_url);
	}

	/* Close Connection with Server */
	close(server_sockfd);

	free(response);
	free(request);
}
