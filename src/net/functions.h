#include "functions/http_receive.h"
#include "functions/http_send.h"
#include "functions/socket_init.h"
#include "functions/socket_receive.h"
#include "functions/socket_send.h"

/* SOCKET APIs */
void socket_client_start (
    struct socket* sock
);

void socket_server_start (
    struct socket* sock
);

void socket_close (
    struct socket* sock
);

/* TCP APIs */
struct tcp tcp_client_start (
    str address,
    int port
);

struct tcp tcp_server_start (
    str address,
    int port
);

/* HTTP APIs */
struct http http_client_start (
    str host
);

struct http http_server_start (
    str host
);

void http_message_print (
    struct http_message* message
);

void http_close (
    struct http* http
);
