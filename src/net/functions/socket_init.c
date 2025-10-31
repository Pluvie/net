int socket_init (
    struct socket* sock,
    str host,
    int port,
    int protocol
)
{
#if   platform(LINUX)
  #include "socket_init_linux.c"

#elif platform(WINDOWS)
  #include "socket_init_windows.c"

#endif
}
