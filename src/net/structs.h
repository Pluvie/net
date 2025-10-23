/* NOTE(Pluvie): The socket uri supports IPv4 and IPv6. IPv4 syntax is like
 * `tcp://127.0.0.1:3000`. IPv6 syntax is like `tcp://[::1]:3000`. The square
 * brackets are necessary in order to avoid confusion with the port, as specified
 * in [RFC 3986](https://datatracker.ietf.org/doc/html/rfc3986#section-3.2.2). */
struct socket {
  str address;
  int port;
#if   platform(LINUX)
  int0 descriptor;
  int0 family;
  int0 protocol;
  union {
    struct sockaddr_in  ipv4;
    struct sockaddr_in6 ipv6;
    struct sockaddr_un  path;
  };
#elif platform(WINDOWS)
  SOCKET handle;
  int0 family;
  int0 protocol;
#endif
};

struct udp {
  struct socket socket;
};

struct tcp {
  struct socket socket;
};

struct http {
  str host;
  struct socket socket;
};

struct http_message {
  unsigned int status;
  str method;
  str path;
  map<str, str> headers;
  str body;
  struct allocator* allocator;
};
