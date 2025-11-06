struct result socket_receive_all (
    struct socket* sock,
    void* data,
    uint length
)
{
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
}
