struct result socket_receive_all (
    struct socket* sock,
    void* data,
    uint length
)
{
#if platform(LINUX)
  /* For platforms that support the MSG_WAITALL flag this implementation is trivial.
    See: https://man7.org/linux/man-pages/man2/recv.2.html */
  struct result result;
  sock->flags |= MSG_WAITALL;
  result = socket_receive(sock, data, length, nullptr);
  sock->flags &= ~MSG_WAITALL;
  return result;

#else
  int total_received_bytes = 0;

  while (total_received_bytes < length) {
    int received_bytes;
    int remaining_bytes = length - total_received_bytes;
    byte* data_offset = ((byte*) data) + total_received_bytes;

    result = socket_receive(sock, data_offset, remaining_bytes, &received_byes);
    if (unlikely(result.failure))
      return result;

    total_received_bytes += received_bytes;
  }

  return succeed();
#endif
}
