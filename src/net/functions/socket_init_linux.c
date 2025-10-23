
  struct socket sock = { .address = address, .port = port };

  /* Check for an empty address. Duh. */
  if (unlikely(str_empty(address))) {
    fail("socket_init: empty address");
    return sock;
  }

  /* Protocol check. Accepts only TCP (SOCK_STREAM) or UDP (SOCK_DGRAM). */
  switch (protocol) {
  case SOCK_STREAM:
  case SOCK_DGRAM:
    sock.protocol = protocol;
    break;
  default:
    fail("socket_init: unsupported protocol");
  }

  /* Depending on the given address, tries to deduce the corresponding address.
    Addresses starting with a digit ('0'..'9') are assumed to be IPv4 -- like 127.0.0.1.
    Addresses starting with a square bracket ('[') are assumed to be IPv6, according to
    [RFC 3986](https://datatracker.ietf.org/doc/html/rfc3986#section-3.2.2).
    Other addresses are assumed to be address names, such as "www.google.com". In this
    case, a DNS solution to resolve its IPv4 or IPv6 address is necessary. */
  switch (*address.chars) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    goto ipv4_address;
  case '[':
    goto ipv6_address;
  default:
    goto resolve_address;
  }

ipv4_address:
  sock.family = AF_INET;

  char ipv4[INET_ADDRSTRLEN] = { 0 };
  prints(ipv4, sizeof(ipv4), f(sock.address));

  sock.ipv4.sin_family = sock.family;
  sock.ipv4.sin_port = htons(sock.port);
  int valid_ipv4_address = inet_pton(sock.family, ipv4, &(sock.ipv4.sin_addr));
  if (unlikely(valid_ipv4_address != 1)) {
    fail("socket_init: invalid IPv4 address `", ipv4, "`");
    return sock;
  }

  goto initialize_descriptor;

ipv6_address:
  sock.family = AF_INET6;

  char ipv6[INET6_ADDRSTRLEN] = { 0 };
  prints(ipv6, sizeof(ipv6), f(sock.address));

  sock.ipv6.sin6_family = sock.family;
  sock.ipv6.sin6_port = htons(sock.port);
  int valid_ipv6_address6 = inet_pton(sock.family, ipv6, &(sock.ipv6.sin6_addr));
  if (unlikely(valid_ipv6_address6 != 1)) {
    fail("socket_init: invalid IPv6 address `", ipv6, "`");
    return sock;
  }

  goto initialize_descriptor;

resolve_address:
  /* Address to be resolved. */
  char address_to_resolve[NI_MAXHOST] = { 0 };
  char port_to_resolve[NI_MAXSERV] = { 0 };
  prints(address_to_resolve, sizeof(address_to_resolve), f(address));
  prints(port_to_resolve, sizeof(port_to_resolve), f(port));
  /* Uncomment to debug name and service resolution.
  printl("name >>> ", address_to_resolve);
  printl("service >>> ", port_to_resolve); */

  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = sock.protocol
  };
  struct addrinfo* results;

  int error = getaddrinfo(address_to_resolve, port_to_resolve, &hints, &results);
  if (unlikely(error != 0)) {
    fail("socket_init: getaddrinfo error. ", gai_strerror(error));
    return sock;
  }

  for (struct addrinfo* res = results; res != nullptr; res = res->ai_next) {
    if (res->ai_family == AF_INET) {
      sock.family = res->ai_family;
      sock.ipv4.sin_family = res->ai_family;
      sock.ipv4.sin_port = htons(sock.port);
      sock.ipv4.sin_addr = ((struct sockaddr_in*) res->ai_addr)->sin_addr;
      /* Uncomment to debug IP resolution.
      char ip[INET_ADDRSTRLEN];
      inet_ntop(sock.family, &(sock.ipv4.sin_addr), ip, sizeof(ip));
      printl("resolved IP: ", ip); */
      break;

    } else if (res->ai_family == AF_INET6) {
      sock.family = res->ai_family;
      sock.ipv6.sin6_family = res->ai_family;
      sock.ipv6.sin6_port = htons(sock.port);
      sock.ipv6.sin6_addr = ((struct sockaddr_in6*) res->ai_addr)->sin6_addr;
      /* Uncomment to debug IP resolution.
      char ip[INET6_ADDRSTRLEN];
      inet_ntop(sock.family, &(sock.ipv6.sin6_addr), ip, sizeof(ip));
      printl("resolved IP: ", ip); */
      break;
    }
  }

  freeaddrinfo(results);
  goto initialize_descriptor;

initialize_descriptor:
  sock.descriptor = socket(sock.family, sock.protocol, 0);
  if (unlikely(sock.descriptor < 0))
    fail("socket_init: internal failure. ", strerror(errno));

  return sock;
