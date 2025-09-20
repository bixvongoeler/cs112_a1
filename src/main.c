#include <stdio.h>
#include <stdlib.h>
#include "proxy.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		return 1;
	}

	int port = atoi(argv[1]);

	if (port <= 0) {
		printf("Invalid port: %d\n", port);
		return 1;
	}

	Proxy proxy = proxy_new(port);
	proxy_run(proxy);
	printf("Proxy Finished");
	proxy_free(&proxy);
	printf("Proxy Memory Freed");

	return 0;
}
