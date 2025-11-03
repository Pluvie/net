#include "functions/http_receive.c"
#include "functions/http_send.c"
#include "functions/socket_init.c"
#include "functions/socket_receive.c"
#include "functions/socket_send.c"

/* SOCKET APIs */
enum result socket_client_start (
    struct socket* sock
)
{
#if platform(LINUX)
  enum result result;

  switch (sock->family) {
  case AF_INET: {
    result = connect(sock->descriptor, &(sock->address.ipv4), sizeof(sock->address.ipv4));
    if (unlikely(result == -1))
      return fail("socket connection failure");
    break;
  }

  case AF_INET6: {
    result = connect(sock->descriptor, &(sock->address.ipv6), sizeof(sock->address.ipv6));
    if (unlikely(result == -1))
      return fail("socket connection failure");
    break;
  }

  default:
    return fail("unsupported socket type");
  }

  return Success;

#elif platform(WINDOWS)
  /* To do. */

#endif
}

enum result socket_server_start (
    struct socket* sock
);



void socket_close (
    struct socket* sock
)
{
  close(sock->descriptor);
}

/* TCP APIs */
enum result tcp_client_start (
    struct tcp* client,
    str host,
    int port
)
{
  enum result result;
  memory_set(client, 0, sizeof(*client));

  result = socket_init(&(client->socket), host, port, SOCK_STREAM);
  if (unlikely(result == Failure))
    return Failure;

  result = socket_client_start(&(client->socket));
  if (unlikely(result == Failure))
    return Failure;

  return Success;
}

enum result tcp_server_start (
    struct tcp* server,
    str host,
    int port
);

/* HTTP APIs */
enum result http_client_start (
    struct http* client,
    str host
)
{
  enum result result;
  memory_set(client, 0, sizeof(*client));
  client->host = host;

  result = socket_init(&(client->socket), host, 80, SOCK_STREAM);
  if (unlikely(result == Failure))
    return Failure;

  result = socket_client_start(&(client->socket));
  if (unlikely(result == Failure))
    return Failure;

  return Success;
}

enum result http_server_start (
    struct http* client,
    str host
);

void http_message_print (
    struct http_message* message
)
{
  print("%"fmt(STR), str_fmt(message->incipit));
  if (!str_empty(message->body))
    print("%"fmt(STR), str_fmt(message->body));

  printl("");
}

void http_close (
    struct http* http
)
{
  socket_close(&(http->socket));
}
