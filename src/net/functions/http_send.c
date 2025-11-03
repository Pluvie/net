#include "http_message_internal.c"

enum result http_send (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  enum result result;
  result = http_message_incipit_build(message, http->host, allocator);
  if (unlikely(result == Failure))
    return Failure;


  result = socket_send(&(http->socket), message->incipit.chars, message->incipit.length, 0);
  if (unlikely(result == Failure))
    return Failure;

  if (str_empty(message->body))
    return Success;

  result = socket_send(&(http->socket), message->body.chars, message->body.length, 0);
  if (unlikely(result == Failure))
    return Failure;

  return Success;
}
