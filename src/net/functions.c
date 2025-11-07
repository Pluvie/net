#include "functions/http_message_internal.c"

#include "functions/http_receive.c"
#include "functions/http_send.c"
#include "functions/socket_init.c"
#include "functions/socket_receive.c"
#include "functions/socket_receive_all.c"
#include "functions/socket_receive_until_str.c"
#include "functions/socket_send.c"

/* SOCKET APIs */
struct result socket_client_start (
    struct socket* sock
)
{
#if platform(LINUX)
  int_t connect_result;

  switch (sock->family) {
  case AF_INET: {
    connect_result = connect(
      sock->descriptor, &(sock->address.ipv4), sizeof(sock->address.ipv4));
    if (unlikely(connect_result == -1))
      return fail_with_errno("socket connection failure");
    else
      return succeed();
  }

  case AF_INET6: {
    connect_result = connect(
      sock->descriptor, &(sock->address.ipv6), sizeof(sock->address.ipv6));
    if (unlikely(connect_result == -1))
      return fail_with_errno("socket connection failure");
    else
      return succeed();
  }

  default:
    break;
  }

  return fail("unsupported socket type");

#elif platform(WINDOWS)
  /* To do. */

#endif
}

struct result socket_server_start (
    struct socket* sock
);

void socket_close (
    struct socket* sock
)
{
  close(sock->descriptor);
}

struct result socket_peek (
    struct socket* sock,
    void* data,
    uint length,
    int* peeked_bytes_ptr
)
{
  struct result result;

#if platform(LINUX)
  sock->flags |= MSG_PEEK;
  result = socket_receive(sock, data, length, peeked_bytes_ptr);
  sock->flags &= ~MSG_PEEK;
  return result;

#elif platform(WINDOWS)
  /* To do. */

#endif
}


/* TCP APIs */
struct result tcp_client_start (
    struct tcp* client,
    str host,
    int port
)
{
  struct result result;
  memory_set(client, 0, sizeof(*client));

  result = socket_init(&(client->socket), host, port, SOCK_STREAM);
  if (unlikely(result.failure))
    return result;

  result = socket_client_start(&(client->socket));
  if (unlikely(result.failure))
    return result;

  return succeed();
}

struct result tcp_server_start (
    struct tcp* server,
    str host,
    int port
);

/* HTTP APIs */
struct result http_client_start (
    struct http* client,
    str host
)
{
  struct result result;
  memory_set(client, 0, sizeof(*client));
  client->host = host;

  result = socket_init(&(client->socket), host, 80, SOCK_STREAM);
  if (unlikely(result.failure))
    return result;

  result = socket_client_start(&(client->socket));
  if (unlikely(result.failure))
    return result;

  return succeed();
}

struct result http_server_start (
    struct http* client,
    str host
);

void http_message_print (
    struct http_message* message
)
{
  if (str_empty(message->method))
    printl("%"fmt(STR)" %"fmt(INT),
      str_fmt(http_chunks.version_1_1), message->status);
  else
    printl("%"fmt(STR)" %"fmt(STR)" %"fmt(STR),
      str_fmt(message->method), str_fmt(message->path), str_fmt(http_chunks.version_1_1));

  { str* name; str* value; struct iterator iter = { 0 };
    while (http_headers_each(&(message->headers), &iter, &name, &value))
      printl("%"fmt(STR)": %"fmt(STR), str_fmt(*name), str_fmt(*value));
  }

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
