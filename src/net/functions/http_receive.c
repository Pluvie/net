#include "http_receive_internal.c"

int http_receive (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  int error;
  cstr buffer = allocator_push(allocator, HTTP_BUFFER_LENGTH);
  /*
  str parser = { 0 };

  memory_set(buffer, 0, parser.length);
  memory_set(message, 0, sizeof(*message));
  parser.chars = buffer;
  parser.length = HTTP_BUFFER_LENGTH;
  */
  error = socket_receive(&(http->socket), HTTP_BUFFER_LENGTH, buffer, 0);
  if (unlikely(error == -1))
    return FAILURE;

  printl("Received:");
  printl("%"fmt(CSTR), buffer);

  return SUCCESS;
}
