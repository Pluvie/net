#include <ion/external.h>

#if platform(LINUX)
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>

#elif platform(WINDOWS)
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32.lib") /* Link Winsock library */

#else
  #error "🌐NET📡: Unsupported platform."
#endif

#include <ion/external.h>
