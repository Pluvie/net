struct socket {
  str host;
  int port;
#if   platform(LINUX)
  int_t descriptor;
  int_t family;
  int_t protocol;
  union {
    struct sockaddr_in  ipv4;
    struct sockaddr_in6 ipv6;
    struct sockaddr_un  path;
  } address;
#elif platform(WINDOWS)
  SOCKET handle;
  int_t family;
  int_t protocol;
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

struct http_body {
  str content;
  /* Add other useful things maybe for multipart? */
};

struct http_message {
  uint status;
  str method;
  str path;
  struct http_headers headers;
  struct http_body body;
  struct allocator* allocator;
};
