#include "functions/http_receive.h"
#include "functions/http_send.h"
#include "functions/socket_init.h"
#include "functions/socket_receive.h"
#include "functions/socket_receive_all.h"
#include "functions/socket_send.h"

/* SOCKET APIs */
Attribute__No_Discard
struct result socket_client_start (
    struct socket* sock
);

Attribute__No_Discard
struct result socket_server_start (
    struct socket* sock
);

void socket_close (
    struct socket* sock
);

struct result socket_peek (
    struct socket* sock,
    void* data,
    uint length,
    int* peeked_bytes_ptr
);

/* TCP APIs */
Attribute__No_Discard
struct result tcp_client_start (
    struct tcp* client,
    str address,
    int port
);

Attribute__No_Discard
struct result tcp_server_start (
    struct tcp* server,
    str address,
    int port
);

/* HTTP APIs */
Attribute__No_Discard
struct result http_client_start (
    struct http* client,
    str host
);

Attribute__No_Discard
struct result http_server_start (
    struct http* server,
    str host
);

void http_message_print (
    struct http_message* message
);

void http_close (
    struct http* http
);
