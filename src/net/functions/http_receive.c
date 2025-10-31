#include "http_receive_internal.c"

bool http_receive (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  cstr buffer = allocator_push(allocator, HTTP_BUFFER_LENGTH);
  str parser = { 0 };

  memory_set(buffer, 0, parser.length);
  memory_set(message, 0, sizeof(*message));
  parser.chars = buffer;
  parser.length = HTTP_BUFFER_LENGTH;

  return true;
}
