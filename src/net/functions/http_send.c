struct result http_send (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  struct result result;
  result = http_message_incipit_build(message, http->host, allocator);
  if (unlikely(result.failure))
    return result;


  result = socket_send(&(http->socket), message->incipit.chars, message->incipit.length, 0);
  if (unlikely(result.failure))
    return result;

  if (str_empty(message->body))
    return succeed();

  result = socket_send(&(http->socket), message->body.chars, message->body.length, 0);
  if (unlikely(result.failure))
    return result;

  return succeed();
}
