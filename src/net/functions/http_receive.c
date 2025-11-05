static struct result http_receive_incipit (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  struct result result;
  str* incipit = &(message->incipit);

  str data, search;
  int index, bytes_to_receive;
  uint allocator_initial_position = allocator->line.position;

  incipit->length = 0;

receive_chunk:
  if (incipit->length > HTTP_INCIPIT_MAXLEN)
    return fail("did not find an incipit terminator");

  data.chars = allocator_stretch(allocator, HTTP_CHUNK_MINSIZE);
  data.length = HTTP_CHUNK_MINSIZE;

  result = socket_peek(&(http->socket), data.chars, HTTP_CHUNK_MINSIZE, (int*) &(data.length));
  if (unlikely(result.failure))
    return result;

  if (incipit->length > 0 && data.length >= http_chunks.terminator.length) {
    search.chars = data.chars - http_chunks.terminator.length;
    search.length = data.length + http_chunks.terminator.length;
  } else {
    search.chars = data.chars;
    search.length = data.length;
  }

  index = str_index(search, http_chunks.terminator);
  if (index >= 0) {
    bytes_to_receive = index - (search.length - data.length) + http_chunks.terminator.length;
    result = socket_receive(&(http->socket), data.chars, bytes_to_receive, nullptr);
    if (unlikely(result.failure))
      return result;

    incipit->length += bytes_to_receive;
    incipit->chars = ((char*) allocator->line.data) + allocator_initial_position;
    return succeed();
  }

  result = socket_receive(&(http->socket), data.chars, HTTP_CHUNK_MINSIZE, nullptr);
  if (unlikely(result.failure))
    return result;

  incipit->length += HTTP_CHUNK_MINSIZE;
  goto receive_chunk;

  return fail("unreachable");
}

struct result http_receive (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  struct result result;

  result = http_receive_incipit(http, message, allocator);
  if (unlikely(result.failure))
    return result;

  result = http_message_incipit_decode(message, allocator);
  if (unlikely(result.failure))
    return result;

  return succeed();
}
