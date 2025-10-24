#include "http_receive_internal.c"

struct http_message* http_receive (
    struct http* http,
    struct allocator* allocator
)
{
  struct http_message* message = allocator_push(allocator, sizeof(struct http_message));
  memory_set(message, 0, sizeof(*message));

  str parser = { allocator_push(allocator, HTTP_BUFFER_LENGTH), HTTP_BUFFER_LENGTH };
  memory_set(parser.chars, 0, parser.length);

  http_parse_inicipt(message, &parser);
  if (unlikely(failure.occurred))
    return nullptr;

  return message;
}
