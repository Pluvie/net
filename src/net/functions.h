#include "functions/http_receive.h"
#include "functions/http_send.h"
#include "functions/socket_init.h"
#include "functions/socket_receive.h"
#include "functions/socket_send.h"

/* SOCKET APIs */
Attribute__No_Discard
int socket_client_start (
    struct socket* sock
);

Attribute__No_Discard
int socket_server_start (
    struct socket* sock
);

void socket_close (
    struct socket* sock
);

/* TCP APIs */
Attribute__No_Discard
int tcp_client_start (
    struct tcp* client,
    str address,
    int port
);

Attribute__No_Discard
int tcp_server_start (
    struct tcp* server,
    str address,
    int port
);

/* HTTP APIs */
Attribute__No_Discard
int http_client_start (
    struct http* client,
    str host
);

Attribute__No_Discard
int http_server_start (
    struct http* server,
    str host
);

void http_message_print (
    struct http_message* message
);

void http_message_set_headers (
    struct http_message* message,
    struct http_header* headers,
    uint length
);

void http_close (
    struct http* http
);
