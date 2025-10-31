int socket_receive (
    struct socket* sock,
    uint length,
    void* data,
    int flags
)
{
#if platform(LINUX)
  int result = recv(sock->descriptor, data, length, flags);
  if (unlikely(result == -1)) {
    fail("socket receive failure");
    return -1;
  }

  return result;

#elif platform(WINDOWS)
  /* To do. */

#endif
}
