#include "functions/http_receive.c"
#include "functions/http_send.c"
#include "functions/socket_init.c"
#include "functions/socket_receive.c"
#include "functions/socket_send.c"

/* SOCKET APIs */
int socket_client_start (
    struct socket* sock
)
{
#if platform(LINUX)
  int result;

  printl("wattafuck family: %"fmt(INT_T), sock->family);
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

  return SUCCESS;

#elif platform(WINDOWS)
  /* To do. */

#endif
}

int socket_server_start (
    struct socket* sock
);



void socket_close (
    struct socket* sock
)
{
  close(sock->descriptor);
}

/* TCP APIs */
int tcp_client_start (
    struct tcp* client,
    str host,
    int port
)
{
  int error;
  memory_set(client, 0, sizeof(*client));

  error = socket_init(&(client->socket), host, port, SOCK_STREAM);
  if (unlikely(error))
    return FAILURE;

  error = socket_client_start(&(client->socket));
  if (unlikely(error))
    return FAILURE;

  return SUCCESS;
}

int tcp_server_start (
    struct tcp* server,
    str host,
    int port
);

/* HTTP APIs */
int http_client_start (
    struct http* client,
    str host
)
{
  int error;
  memory_set(client, 0, sizeof(*client));
  client->host = host;

  error = socket_init(&(client->socket), host, 80, SOCK_STREAM);
  if (unlikely(error))
    return FAILURE;

  error = socket_client_start(&(client->socket));
  if (unlikely(error))
    return FAILURE;

  return SUCCESS;
}

int http_server_start (
    struct http* client,
    str host
);

void http_message_print (
    struct http_message* message
)
{
  printl("Method: %"fmt(STR), str_fmt(message->method));
  printl("Path: %"fmt(STR), str_fmt(message->path));
  printl("Headers:");
  {
    str* key; str* value; struct iterator iter = { 0 };
    while (http_headers_each(&(message->headers), &iter, &key, &value))
      printl("  %"fmt(STR)": %"fmt(STR), str_fmt(*key), str_fmt(*value));
  }
  if (str_empty(message->body.content))
    return;
  printl("Body:\n%"fmt(STR), str_fmt(message->body.content));
}

void http_message_set_headers (
    struct http_message* message,
    struct http_header* headers,
    uint length
)
{
  uint i; for (i = 0; i < length; i++) {
    struct http_header* header = headers + i;
    http_headers_set(&(message->headers), header->key, header->value);
  }
}


void http_close (
    struct http* http
)
{
  socket_close(&(http->socket));
}
