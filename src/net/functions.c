#include "functions/http_receive.c"
#include "functions/http_send.c"
#include "functions/socket_init.c"
#include "functions/socket_receive.c"
#include "functions/socket_send.c"

/* SOCKET APIs */
bool socket_client_start (
    struct socket* sock
)
{
#if platform(LINUX)
  int result;

  switch (sock->family) {
  case AF_INET: {
    result = connect(sock->descriptor, &(sock->ipv4), sizeof(sock->ipv4));
    if (result == -1)
      return fail("socket connection failure");
  }

  case AF_INET6: {
    result = connect(sock->descriptor, &(sock->ipv6), sizeof(sock->ipv6));
    if (result == -1)
      return fail("socket connection failure");
  }

  default:
    return fail("unsupported socket type");
  }

  return true;

#elif platform(WINDOWS)
  /* To do. */

#endif
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
