#include "functions/http_receive.c"
#include "functions/http_send.c"
#include "functions/socket_init.c"
#include "functions/socket_receive.c"
#include "functions/socket_send.c"

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



void socket_close (
    struct socket* sock
)
{
  close(sock->descriptor);
}

/* TCP APIs */
struct tcp tcp_client_start (
    str address,
    int port
)
{
  struct tcp client = { 0 };
  client.socket = socket_init(address, port, SOCK_STREAM);
  if (unlikely(failure.occurred))
    return client;

  socket_client_start(&(client.socket));
  return client;
}

struct tcp tcp_server_start (
    str address,
    int port
);

/* HTTP APIs */
struct http http_client_start (
    str host
)
{
  struct http client = { .host = host };
  client.socket = socket_init(host, 80, SOCK_STREAM);
  if (unlikely(failure.occurred))
    return client;

  socket_client_start(&(client.socket));
  return client;
}

struct http http_server_start (
    str host
);

void http_message_print (
    struct http_message* message
)
{
  printl("Method: ", f(message->method));
  printl("Path: ", f(message->path));
  printl("Headers:");
  for map_each(&(message->headers), str* name, str* value)
    printl("  ", f(*name), ": ", f(*value));
  if (str_empty(message->body))
    return;
  printl("Body:\n", f(message->body));
}


void http_close (
    struct http* http
)
{
  socket_close(&(http->socket));
}
