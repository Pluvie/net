struct socket socket_init (
    str uri,
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
