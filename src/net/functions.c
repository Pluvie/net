/* SOCKET APIs */
struct socket socket_init (
    str uri,
    str proto
)
{
  struct socket sock = { .uri = uri };

  if (unlikely(str_empty(uri))) {
    fail("socket_init: empty uri");
    return sock;
  }

  if (str_equal(uri, string("file://"))) {
    /* Local socket. */
    socket.family = LOCAL;

  } else {
    /* Internet socket. */
    switch (*uri.chars) {
    case '[':
      /* IPv6 address. */
      socket.family = IP_V6;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      /* IPv4 address. */
      socket.family = IP_V4;

    default:
      /* Hostname to be resolved. */
    }
  }

  if (str_equal(proto, string("tcp")))
    sock.protocol = TCP;
  else if (str_equal(proto, string("udp")))
    sock.protocol = UDP;

  sock.descriptor = socket(sock.family, sock.protocol, 0);
  if (unlikely(sock.descriptor < 0))
    fail("socket_init: internal failure");

  return sock;
}

void socket_client_start (
    struct socket* sock
);

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
);

/* TCP APIs */
struct tcp tcp_client_start (
    str uri
)
{
  struct tcp tcp = { 0 };
  tcp.socket = socket_init(uri, string("tcp"));
  if (unlikely(failure.occurred))
    return tcp;

  
  return tcp;
}

struct tcp tcp_server_start (
    str uri
);

/* HTTP APIs */
struct http http_client_start (
    str uri
)
{
  struct http client = { .uri = uri, .tcp = tcp_client_start(uri) };
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
