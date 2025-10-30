int socket_send (
    struct socket* sock,
    unsigned int length,
    void* data,
    int flags
)
{
#if platform(LINUX)
  int result = send(sock->descriptor, data, length, flags);
  if (unlikely(result == -1)) {
    fail("socket_send: ", strerror(errno));
    return -1;
  }
  return result;

#elif platform(WINDOWS)
  /* To do. */

#endif
}
