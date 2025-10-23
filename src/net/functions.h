#include "functions/socket_init.h"

/* SOCKET APIs */
void socket_client_start (
    struct socket* sock
);

void socket_server_start (
    struct socket* sock
);

void socket_receive (
    struct socket* sock,
    unsigned int length,
    void* address
);

void socket_send (
    struct socket* sock,
    unsigned int length,
    void* address
);

void socket_close (
    struct socket* sock
);

/* TCP APIs */
struct tcp tcp_client_start (
    str uri,
    int port
);

struct tcp tcp_server_start (
    str uri,
    int port
);

/* HTTP APIs */
struct http http_client_start (
    str uri
);

struct http http_server_start (
    str uri
);

struct http_message* http_receive (
    struct http* http,
    struct allocator* allocator
);

void http_send (
    struct http* server,
    struct http_message* message
);

void http_close (
    struct http* http
);
