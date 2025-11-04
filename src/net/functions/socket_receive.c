struct result socket_receive (
    struct socket* sock,
    void* data,
    uint length,
    int* received_bytes_ptr
)
{
#if platform(LINUX)
  int received_bytes = recv(sock->descriptor, data, length, sock->flags);
  if (unlikely(received_bytes == -1))
    return fail("socket receive failure");

  printl("-----------------------");
  printl("Received: [%"fmt(UINT)"]", length);
  print_hex(data, length);
  printl("-----------------------");
  if (received_bytes_ptr != nullptr)
    *received_bytes_ptr = received_bytes;

  return succeed();

#elif platform(WINDOWS)
  /* To do. */

#endif
}
