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

  result = socket_peek(&(http->socket),
    data.chars, HTTP_CHUNK_MINSIZE, (int*) &(data.length));
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
    bytes_to_receive = index - (search.length - data.length) +
      http_chunks.terminator.length;
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

static struct result http_receive_body (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  struct result result;

  if (http_headers_has(&(message->headers), http_headers.content_length))
    goto receive_body_entirely;

  if (http_headers_has(&(message->headers), http_headers.transfer_encoding)) {
    str chunked = str("chunked");
    str* transfer_encoding = http_headers_get(&(message->headers),
      http_headers.transfer_encoding);
    if (str_contains(*transfer_encoding, chunked))
      goto receive_body_chunked;
  }

  /* No body to receive. */
  return succeed();

receive_body_entirely: {
  str* content_length;
  int content_length_number;

  content_length = http_headers_get(&(message->headers), http_headers.content_length);
  result = str_to_int(*content_length, &content_length_number);
  if (unlikely(result.failure))
    return result;

  message->body.chars = allocator_push(allocator, content_length_number);
  message->body.length = content_length_number;

  result = socket_receive_all(&(http->socket),
    message->body.chars, message->body.length);
  if (unlikely(result.failure))
    return result;

  return succeed();
}

receive_body_chunked: {
  int chunk_size, chunk_size_bytes;
  char chunk_size_buffer[INT_MAXNUM] = { 0 };
  result = socket_peek(&(http->socket),
    chunk_size_buffer, sizeof(chunk_size_buffer), &chunk_size_bytes);
  if (unlikely(result.failure))
    return result;

  result = str_to_int(chunk_size_buffer, &chunk_size);
  if (unlikely(result.failure))
    return result;

  message->body.chars = allocator_stretch(allocator, chunk_size);
  message->body.length += chunk_size;

  result = socket_receive_all(&(http->socket),
    message->body.chars, message->body.length);
  if (unlikely(result.failure))
    return result;
}
  
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

  result = http_receive_body(http, message, allocator);
  if (unlikely(result.failure))
    return result;

  return succeed();
}
