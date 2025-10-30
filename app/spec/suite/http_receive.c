#include "http_receive.h"

spec( http_receive ) {
  argument(struct http* http);
  argument(struct allocator* allocator);

  precondition("a valid http server");
  precondition("a valid allocator");
  #define preconditions \
    http = allocator_push(spec_allocator, sizeof(*http)); \
    allocator = spec_allocator;

  when("the message is empty") {
    sim.message = (str) { 0 };
    apply(preconditions);

    http_receive(http, allocator);

    must("read block until timeout");
      verify(sim.timeout == true);

    success();
  } end();

  when("the message does not have a valid incipit") {
    sim.message = string(
      "BLURGHBLURGH"
    );
    apply(preconditions);

    http_receive(http, allocator);

    must("fail with a specific error");
      verify(failure.occurred == true);
      verify(cstr_equal(failure.message,
        "http_receive: unexpected response incipit: BLURGHBLU"));

    success();
  } end();

  when("the message does not have a valid incipit") {
    sim.message = string(
      "BLAH BLAH 200 OK"                      "\r\n"
      "Date: Tue, 15 Nov 1994 08:12:31 GMT"   "\r\n"
      "Content-Type: text/plain"              "\r\n"
      "Content-Length: 12"                    "\r\n"
      ""                                      "\r\n"
      "Hello World!"
    );
    apply(preconditions);

    http_receive(http, allocator);

    must("fail with a specific error");
      verify(failure.occurred == true);
      verify(cstr_equal(failure.message,
        "http_receive: unexpected response incipit: BLAH BLAH"));

    success();
  } end();

  when("the message has a valid incipit with http version but not a valid status") {
    sim.message = string(
      "HTTP/1.1 A"
    );
    apply(preconditions);

    http_receive(http, allocator);

    must("fail with a specific error");
      verify(failure.occurred == true);
      verify(cstr_equal(failure.message,
        "http_receive: unexpected status code: A"));

    success();
  } end();

  when("the message has a valid incipit with http version but not a valid status") {
    sim.message = string(
      "HTTP/1.1 AAA OK"                       "\r\n"
      "Date: Tue, 15 Nov 1994 08:12:31 GMT"   "\r\n"
      "Content-Type: text/plain"              "\r\n"
      "Content-Length: 12"                    "\r\n"
      ""                                      "\r\n"
      "Hello World!"
    );
    apply(preconditions);

    http_receive(http, allocator);

    must("fail with a specific error");
      verify(failure.occurred == true);
      verify(cstr_equal(failure.message,
        "http_receive: unexpected status code: AAA "));

    success();
  } end();

  when("the message has a valid incipit with method") {
    sim.message = string(
      "GET /abc OK"                           "\r\n"
      "Date: Tue, 15 Nov 1994 08:12:31 GMT"   "\r\n"
      "Content-Type: text/plain"              "\r\n"
      "Content-Length: 12"                    "\r\n"
      ""                                      "\r\n"
      "Hello World!"
    );
    apply(preconditions);

    http_receive(http, allocator);

    must("fail with a specific error");
      verify(failure.occurred == true);
      verify(cstr_equal(failure.message,
        "http_receive: unexpected status code: AAA "));

    success();
  } end();
}
