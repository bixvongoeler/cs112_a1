#ifndef PROXY_H
#define PROXY_H

#include "http_parser.h"
#include "socket_utils.h"

void error(char *msg);

typedef struct Proxy *Proxy;

/* create and delete */
Proxy proxy_new(int port);
void proxy_free(Proxy *proxy_ptr);

/* Run the proxy server */
void proxy_run(Proxy proxy);

#endif
