#include "http_parser.h"

#define FIVE_KB 5120
#define TEN_MB 10485760
#define HOUR_IN_SECONDS 3600
#define DEFAULT_PORT 80

void parse_url(char *url, HTTPRequest *req);
void parse_host(char *host, HTTPRequest *req);

/*
 * Read the full HTTP message from the client socket and return it as a string.
 */
char *read_http_request(int client_fd, size_t *request_size)
{
	char *buffer = malloc(5120); // Requests are usually small
	size_t total_read = 0;
	size_t buffer_size = 5120;

	// For GET requests, we only need to read until \r\n\r\n
	while (1) {
		ssize_t bytes_read = recv(client_fd, buffer + total_read,
					  buffer_size - total_read - 1, 0);

		if (bytes_read <= 0) {
			error("ERROR reading from socket");
			free(buffer);
			return NULL;
		}

		total_read += bytes_read;
		buffer[total_read] = '\0';

		// Check if we have complete headers
		if (strstr(buffer, "\r\n\r\n")) {
			*request_size = total_read;
			return buffer;
		}

		// Expand buffer if needed
		if (total_read >= buffer_size - 1) {
			buffer_size *= 2;
			buffer = realloc(buffer, buffer_size);
		}
	}
}

/*
 * Read the full HTTP message from the client socket and return it as a string.
 */
char *read_http_response(int server_sockfd, size_t *response_size)
{
	char *buffer = malloc(TEN_MB); // Requests are usually small
	size_t total_read = 0;
	size_t buffer_size = TEN_MB;

	// Read untill \r\n\r\n, then parse content length and continue.
	char *found_blank = NULL;
	while (found_blank == NULL) {
		ssize_t bytes_read = recv(server_sockfd, buffer + total_read,
					  buffer_size - total_read - 1, 0);

		if (bytes_read <= 0) {
			error("ERROR reading from socket");
			exit(EXIT_FAILURE);
		}

		total_read += bytes_read;
		buffer[total_read] = '\0';

		// Check if we have complete headers
		found_blank = strstr(buffer, "\r\n\r\n");

		// Expand buffer if needed
		if (total_read >= buffer_size - 1) {
			buffer_size *= 2;
			buffer = realloc(buffer, buffer_size);
		}
	}

	/* Parse  */
	char *content_length_key = strstr(buffer, "Content-Length: ");
	if (content_length_key == NULL) {
		error("Content-Length header not found");
		exit(EXIT_FAILURE);
	}
	size_t content_length =
		strtol(content_length_key + strlen("Content-Length: "), NULL, 10);

	size_t header_size = found_blank - buffer + 4;
	size_t content_remaining = content_length - total_read + header_size;
	printf("Content-Length: %zu\n", content_length);

	while (content_remaining != 0) {
		ssize_t bytes_read = recv(server_sockfd, buffer + total_read,
					  buffer_size - total_read - 1, 0);
		printf("Content Remaining: [%zu] - Bytes Read: [%ld]\n",
		       content_remaining, bytes_read);
		if (bytes_read <= 0) {
			error("ERROR reading from socket");
			exit(EXIT_FAILURE);
		}

		total_read += bytes_read;
		content_remaining -= bytes_read;
		buffer[total_read] = '\0';

		// Expand buffer if needed
		if (total_read >= buffer_size - 1) {
			buffer_size *= 2;
			buffer = realloc(buffer, buffer_size);
		}
	}
	printf("Content Remaining At End: [%zu]\n", content_remaining);
	printf("Final Total Read: [%zu]\n", total_read);
	printf("Total Content Read: [%zu]\n", total_read - header_size);
	*response_size = total_read;
	return buffer;
}

/*
 * Parse the HTTP request and return a struct containing the parsed information.
 */

HTTPRequest parse_http_request(char *request_buffer)
{
	HTTPRequest req = { 0 };
	req.port = 80; // Default port

	/* Duplicate request line */
	char *line_break = strchr(request_buffer, '\n') + 1;
	char request_line[512];
	strncpy(request_line, request_buffer, line_break - request_buffer);

	/* Get Info from Request Line */
	char *method = strtok(request_line, " ");
	strcpy(req.method, method);
	char *url = strtok(NULL, " ");
	char *version = strtok(NULL, "\r");
	(void)version;

	/* Handle URL */
	parse_url(url, &req);

	/* Parse Host Header Only */
	char *hostkey = strstr(request_buffer, "Host: ");
	if (hostkey == NULL) {
		printf("No Host Field Found");
		assert(0);
	}
	parse_host(hostkey + 6, &req);

	snprintf(req.full_url, sizeof(req.full_url), "http://%s:%d%s", req.host,
		 req.port, req.path);

	return req;
}

void parse_url(char *url, HTTPRequest *req)
{
	// http://www.cs.tufts.edu/comp/112/index.html
	char *http_exists = strstr(url, "http://");
	if (http_exists && http_exists == url) {
		/* get components */
		char *host = url + 7;
		char *path_start = strchr(host, '/');
		strcpy(req->path, path_start);
	} else {
		// handle relative url
		strcpy(req->path, url);
	}
}

void parse_host(char *host, HTTPRequest *req)
{
	char *line_end = strchr(host, '\r');
	char host_str[256] = { 0 };
	strncpy(host_str, host, line_end - host);
	// Look for colon (port separator)
	char *colon = strchr(host_str, ':');

	if (colon) {
		// Has port
		size_t host_len = colon - host_str;
		strncpy(req->host, host_str, host_len);
		req->host[host_len] = '\0';
		req->port = atoi(colon + 1);
	} else {
		// No port, copy until end of line or space
		size_t host_len = line_end - host;
		strncpy(req->host, host, host_len);
		req->host[host_len] = '\0';
	}
}

int parse_cache_control_max_age(const char *response)
{
	if (!response)
		return HOUR_IN_SECONDS;

	char *cache_control = strstr(response, "Cache-Control:");
	if (!cache_control) {
		cache_control = strstr(response, "cache-control:");
	}

	if (!cache_control)
		return HOUR_IN_SECONDS;

	char *max_age = strstr(cache_control, "max-age=");
	if (!max_age)
		return HOUR_IN_SECONDS;

	char *line_end = strchr(max_age, '\r');
	if (!line_end)
		line_end = strchr(max_age, '\n');
	if (!line_end)
		return HOUR_IN_SECONDS;

	int age = atoi(max_age + 8);
	return age > 0 ? age : HOUR_IN_SECONDS;
}

char *inject_age_header(const char *response, size_t response_size,
			uint64_t age_seconds, size_t *new_size)
{
	if (!response || !new_size)
		return NULL;

	char *blank_line = strstr(response, "\r\n\r\n");
	if (!blank_line)
		return NULL;

	size_t header_end_pos = blank_line - response + 2;

	char age_header[64];
	snprintf(age_header, sizeof(age_header), "Age: %llu\r\n",
		 (unsigned long long)age_seconds);

	size_t age_header_len = strlen(age_header);
	*new_size = response_size + age_header_len;

	char *new_response = malloc(*new_size + 1);
	if (!new_response)
		return NULL;

	memcpy(new_response, response, header_end_pos);

	memcpy(new_response + header_end_pos, age_header, age_header_len);

	memcpy(new_response + header_end_pos + age_header_len,
	       response + header_end_pos, response_size - header_end_pos);

	new_response[*new_size] = '\0';

	return new_response;
}
