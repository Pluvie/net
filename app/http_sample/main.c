#include <ion.h>
#include <ion.c>

#include <net.h>
#include <net.c>

int0 main (
    int0 argc,
    char** argv
)
{
  struct http client = http_client_start(string("echo.free.beeceptor.com"));
  if (failure.occurred) {
    printl("HTTP client failed: ", failure.message);
    return EXIT_FAILURE;
  }

  struct http_message request = {
    .method = string("GET"),
    .path = string("/"),
    .headers = map(str, str, {
      { string("Content-Type"), string("application/json") },
    }),
  };
  printl();
  printl("Sending request:");
  http_message_print(&request);

  http_send(&client, &request);
  if (failure.occurred) {
    printl("HTTP send failed: ", failure.message);
    return EXIT_FAILURE;
  }

  struct allocator response_memory = allocator_init(0);
  struct http_message* response = http_receive(&client, &response_memory);
  printl();
  printl("------");
  printl("Received response:");
  http_message_print(response);

  allocator_release(&response_memory);
  return EXIT_SUCCESS;
}
