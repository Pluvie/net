#include <net.h>
#include <net.c>

void get (
    struct http* client,
    struct allocator* allocator
)
{
  struct http_message get_request = {
    .method = string("GET"),
    .path = string("/"),
    .headers = map(str, str, {
      { string("User-Agent"), string("libnet") },
    }),
  };
  printl();
  printl(">>>>>>>");
  http_message_print(&get_request);

  http_send(client, &get_request);

  if (failure.occurred) {
    printl("HTTP send failed: ", failure.message);
    return;
  }

  http_receive(client, allocator);
  //struct http_message* response = http_receive(client, allocator);
  //printl();
  //printl("<<<<<<<");
  //printl("Received response:");
  //http_message_print(response);
}

void post (
    struct http* client,
    struct allocator* allocator
)
{
  struct http_message post_request = {
    .method = string("POST"),
    .path = string("/create"),
    .headers = map(str, str, {
      { string("User-Agent"), string("libnet") },
      { string("Content-Type"), string("application/json") },
    }),
    .body = string(
      "{"                                     "\n"
      "  \"integer\": 12345,"                 "\n"
      "  \"decimal\": 777.7E-30,"             "\n"
      "  \"string\": \"value\","              "\n"
      "  \"boolean\": true,"                  "\n"
      "  \"strings\": ["                      "\n"
      "    \"string 1\","                     "\n"
      "    \"string 2\","                     "\n"
      "    null,"                             "\n"
      "    \"string 4\""                      "\n"
      "  ],"                                  "\n"
      "  \"squad\": {"                        "\n"
      "    \"garrus\": {"                     "\n"
      "      \"name\": \"Garrus Vakarian\","  "\n"
      "      \"class\": 2,"                   "\n"
      "      \"health\": 200,"                "\n"
      "      \"shields\": 400"                "\n"
      "    },"                                "\n"
      "    \"wrex\": {"                       "\n"
      "      \"name\": \"Urdnot Wrex\","      "\n"
      "      \"class\": 1,"                   "\n"
      "      \"health\": 800,"                "\n"
      "      \"shields\": 800"                "\n"
      "    }"                                 "\n"
      "  },"                                  "\n"
      "  \"numbers\": ["                      "\n"
      "    0,"                                "\n"
      "    1,"                                "\n"
      "    2,"                                "\n"
      "    3"                                 "\n"
      "  ],"                                  "\n"
      "  \"matrix\": ["                       "\n"
      "    1.1, 1.1, 1.1,"                    "\n"
      "    2.2, 2.2, 2.2,"                    "\n"
      "    3.3, 3.3, 3.3"                     "\n"
      "  ]"                                   "\n"
      "}"
    ),
  };
  printl();
  printl(">>>>>>>");
  http_message_print(&post_request);
  http_send(client, &post_request);

  http_receive(client, allocator);
  //printl();
  //printl("<<<<<<<");
  //http_message_print(&post_request);
}

int0 main (
    int0 argc,
    char** argv
)
{
  str host = string("echo.free.beeceptor.com");
  struct http client = http_client_start(host);
  if (unlikely(failure.occurred)) {
    printl("HTTP client failed: ", failure.message);
    return EXIT_FAILURE;
  }

  struct allocator response_memory = allocator_init(0);

  //get(&client, &response_memory);
  post(&client, &response_memory);

  allocator_release(&response_memory);
  http_close(&client);
  return EXIT_SUCCESS;
}
