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

static inline str http_receive_status (
    struct http* http,
    struct allocator* allocator
)
{
  int buffer_length = 8192;
  char* buffer = allocator_push(buffer_length);
  memory_set(buffer, 0, buffer_length);

  int read_bytes = socket_receive(&(http->socket), buffer_length - 1, buffer, 0);
  if (unlikely(failure.occurred))
    return 0;

  str cursor = { buffer, buffer_length }; 

  str expected_incipit = string("HTTP/1.1 ");
  str incipit = { cursor, expected_incipit.length };
  if (!str_equal(incipit, expected_incipit)) {
    fail("http_receive: unexpected response incipit: ", f(incipit));
    return cursor;
  }
  cursor.length += incipit.length;

  str status_code = { cursor.chars, 4 };
  unsigned int status = str_to_int(&status_code);
  if (unlikely(failure.occurred)) {
    fail("http_receive: unexpected status code: ", f(status_code));
    return cursor;
  }
  if (unlikely(status < 100 || status > 599)) {
    fail("http_receive: unexpected status code: ", f(status));
    return cursor;
  }
  cursor.chars += status_code.length;
  message->status = status;

  str expected_crlf = string("\r\n");
  while (*cursor.chars != 0) {
    str crlf = { cursor.chars, expected_crfl.length };
    if (str_equal(crlf, expected_crlf))
      return (str) { cursor.chars, cursor.chars - buffer };
    cursor.chars++;
  }

  fail("http_receive: incipit too long");
  return cursor;
}

struct http_message* http_receive (
    struct http* http,
    struct allocator* allocator
)
{
  struct http_message* message = allocator_push(sizeof(struct http_message));
  memory_set(message, 0, sizeof(*message));

  str cursor = http_receive_status(http);

  return message;
}

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
