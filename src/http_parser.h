#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include "socket_utils.h"
#include "stdbool.h"

typedef struct {
	char method[10]; // "GET"
	char full_url[256]; // Full URL for caching key
	char host[100]; // Hostname to connect to
	char path[256]; // Path to request from server
	int port; // Port to connect to (default 80)
	char *raw_request; // Store the full request to forward
	size_t request_size; // Size of the full request
} HTTPRequest;

char *read_http_request(int client_sockfd, size_t *restrict_size);
char *read_http_response(int server_sockfd, size_t *response_size);
HTTPRequest parse_http_request(char *request_buffer);

int parse_cache_control_max_age(const char *response);
char *inject_age_header(const char *response, size_t response_size,
			uint64_t age_seconds, size_t *new_size);

#endif
