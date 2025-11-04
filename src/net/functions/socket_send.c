struct result socket_send (
    struct socket* sock,
    void* data,
    uint length,
    int* sent_bytes_ptr
)
{
#if platform(LINUX)
  int sent_bytes = send(sock->descriptor, data, length, sock->flags);
  if (unlikely(sent_bytes == -1))
    return fail("socket_send failure");

  if (sent_bytes_ptr != nullptr)
    *sent_bytes_ptr = sent_bytes;

  return succeed();

#elif platform(WINDOWS)
  /* To do. */

#endif
}
