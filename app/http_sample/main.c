#include <net.h>
#include <net.c>

struct result get (
    struct http* client,
    struct allocator* allocator
)
{
  struct result result;

  { /* Firing off the request. */
    struct http_message request = { str("GET"), str("/") };
    struct http_header headers[] = {
      { str("User-Agent"), str("libnet") },
    };

    request.headers = http_headers_alloc(16, allocator);
    { uint index; for (index = 0; index < countof(headers); index++)
      http_headers_set(&(request.headers), headers[index].name, headers[index].value);
    }

    printl("");
    printl(">>>>>>>");
    printl("Sending request:");
    http_message_print(&request);

    result = http_send(client, &request, allocator);
    if (unlikely(result.failure))
      return print_failure("Failed to send HTTP request", &result);

  }

  { /* Receiving the response. */
    struct http_message response = { 0 };

    result = http_receive(client, &response, allocator);
    if (unlikely(result.failure))
      return print_failure("Failed to receive HTTP request", &result);

    printl("");
    printl("<<<<<<<");
    printl("Received response:");
    http_message_print(&response);
  }

  return succeed();
}

struct result post (
    struct http* client,
    struct allocator* allocator
)
{
/*
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
  */

  return succeed();
}

int_t main (
    int_t argc,
    cstr* argv
)
{
  struct result result;
  struct http client = { 0 };
  struct allocator allocator = allocator_init(0);
  str host = str("echo.free.beeceptor.com");

  result = http_client_start(&client, host);
  if (unlikely(result.failure)) {
    print_failure("HTTP client failed", &result);
    return EXIT_FAILURE;
  }

  result = get(&client, &allocator);
  if (unlikely(result.failure))
    goto close;

  /*
  result = post(&client, &allocator);
  if (unlikely(result.failure))
    goto close;
  */

close:
  allocator_release(&allocator);
  http_close(&client);
  return EXIT_SUCCESS;
}
