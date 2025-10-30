#include <ion/allow_include.h>

#if platform(LINUX)
  #include <arpa/inet.h>
  #include <errno.h>
  #include <netdb.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <sys/un.h>
  #include <unistd.h>

#elif platform(WINDOWS)
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32.lib")

#else
  #error "🌐NET📡: Unsupported platform."
#endif

#include <ion/allow_include.h>
