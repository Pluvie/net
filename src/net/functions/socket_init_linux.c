
  /* Whether we have an error or a valid address. */
  int error, address;

  /* Used to get a cstr representation of the host string. */
  char ipv4[INET_ADDRSTRLEN] = { 0 };
  char ipv6[INET6_ADDRSTRLEN] = { 0 };

  /* Used to get a cstr representation of the host string and resolve it via DNS. */
  char address_to_resolve[NI_MAXHOST] = { 0 };
  char port_to_resolve[NI_MAXSERV] = { 0 };
  struct addrinfo hints = { 0 };
  struct addrinfo* results;
  struct addrinfo* current;

  /* Check for an empty host. Duh. */
  if (unlikely(str_empty(host)))
    return fail("empty host");

  memory_set(sock, 0, sizeof(*sock));
  sock->host = host;
  sock->port = port;

  /* Protocol check. Accepts only TCP (SOCK_STREAM) or UDP (SOCK_DGRAM). */
  switch (protocol) {
  case SOCK_STREAM:
  case SOCK_DGRAM:
    sock->protocol = protocol;
    break;
  default:
    return fail("unsupported protocol");
  }

  /* Depending on the given host, tries to resolve the corresponding address.

    - Hosts starting with a digit ('0'..'9') are assumed to be IPv4 -- like
      127.0.0.1.
    - Hosts starting with a square bracket ('[') are assumed to be IPv6, according
      to [RFC 3986](https://datatracker.ietf.org/doc/html/rfc3986#section-3.2.2).
    - Other Hosts are assumed to be DNS names, such as "www.mysite.com". In this case,
      a DNS solution to resolve its IPv4 or IPv6 address is necessary. */
  switch (*host.chars) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    goto ipv4_address;
  case '[':
    goto ipv6_address;
  default:
    goto resolve_address;
  }

ipv4_address:
  memory_copy(ipv4, host.chars, sizeof(ipv4) - 1);
  sock->family = AF_INET;
  sock->address.ipv4.sin_family = sock->family;
  sock->address.ipv4.sin_port = htons(sock->port);
  address = inet_pton(sock->family, ipv4, &(sock->address.ipv4.sin_addr));
  if (unlikely(address != 1))
    return fail("invalid IPv4 address");

  goto initialize_descriptor;

ipv6_address:
  memory_copy(ipv6, host.chars, sizeof(ipv6) - 1);
  sock->family = AF_INET6;
  sock->address.ipv6.sin6_family = sock->family;
  sock->address.ipv6.sin6_port = htons(sock->port);
  address = inet_pton(sock->family, ipv6, &(sock->address.ipv6.sin6_addr));
  if (unlikely(address != 1))
    return fail("invalid IPv6 address");

  goto initialize_descriptor;

resolve_address:
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = sock->protocol;
  memory_copy(address_to_resolve, host.chars, sizeof(address_to_resolve) - 1);
  memory_copy(port_to_resolve, host.chars, sizeof(port_to_resolve) - 1);
  error = getaddrinfo(address_to_resolve, port_to_resolve, &hints, &results);
  if (unlikely(error != 0))
    return fail(gai_strerror(error));

  for (current = results; current != nullptr; current = current->ai_next) {
    if (current->ai_family == AF_INET) {
      sock->family = current->ai_family;
      sock->address.ipv4.sin_family = current->ai_family;
      sock->address.ipv4.sin_port = htons(sock->port);
      sock->address.ipv4.sin_addr = ((struct sockaddr_in*) current->ai_addr)->sin_addr;
      break;

    } else if (current->ai_family == AF_INET6) {
      sock->family = current->ai_family;
      sock->address.ipv6.sin6_family = current->ai_family;
      sock->address.ipv6.sin6_port = htons(sock->port);
      sock->address.ipv6.sin6_addr = ((struct sockaddr_in6*) current->ai_addr)->sin6_addr;
      break;
    }
  }

  freeaddrinfo(results);
  goto initialize_descriptor;

initialize_descriptor:
  sock->descriptor = socket(sock->family, sock->protocol, 0);
  if (unlikely(sock->descriptor < 0))
    return fail("socket internal failure");

  return SUCCESS;
