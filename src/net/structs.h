/* NOTE(Pluvie): The socket uri supports IPv4 and IPv6. IPv4 syntax is like
 * `tcp://127.0.0.1:3000`. IPv6 syntax is like `tcp://[::1]:3000`. The square
 * brackets are necessary in order to avoid confusion with the port, as specified
 * in [RFC 3986](https://datatracker.ietf.org/doc/html/rfc3986#section-3.2.2). */
struct socket {
  str uri;
  enum socket_families family;
  enum socket_protocols protocol;
  union {
    int0 descriptor;  /* For POSIX socket systems. */
    uint0 handle;     /* For Winsock systems. */
  };
};

struct udp {
  struct socket socket;
};

struct tcp {
  struct socket socket;
};

struct http {
  str uri;
  struct tcp tcp;
};

struct http_message {
  str method;
  str path;
  map<str, str> headers;
  str body;
  struct allocator* allocator;
};
