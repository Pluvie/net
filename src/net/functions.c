#include "functions/socket_init.c"

/* SOCKET APIs */
void socket_client_start (
    struct socket* sock
)
{
  switch (sock->family) {
  case AF_INET: {
    int result = connect(sock->descriptor, &(sock->ipv4), sizeof(sock->ipv4));
    if (result == -1)
      fail("socket_client_start: connect failure. ", strerror(errno));
    break;
  }

  case AF_INET6: {
    int result = connect(sock->descriptor, &(sock->ipv6), sizeof(sock->ipv6));
    if (result == -1)
      fail("socket_client_start: connect failure. ", strerror(errno));
    break;
  }

  default:
    fail("socket connect error: unsupported socket type");
  }

  return;
}

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
)
{
  close(sock->descriptor);
}

/* TCP APIs */
struct tcp tcp_client_start (
    str uri,
    int port
)
{
  struct tcp client = { 0 };
  client.socket = socket_init(uri, port, SOCK_STREAM);
  if (unlikely(failure.occurred))
    return client;

  socket_client_start(&(client.socket));
  return client;
}

struct tcp tcp_server_start (
    str uri,
    int port
);

/* HTTP APIs */
struct http http_client_start (
    str uri
)
{
  struct http client = { 0 };
  client.socket = socket_init(uri, 80, SOCK_STREAM);
  if (unlikely(failure.occurred))
    return client;

  socket_client_start(&(client.socket));
  return client;
}

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
)
{
  socket_close(&(http->socket));
}
