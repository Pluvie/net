static struct result http_receive_incipit (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  struct result result = socket_receive_until_str(&(http->socket),
    http_chunks.terminator, &(message->incipit), HTTP_INCIPIT_MAXLEN, allocator);

  if (unlikely(result.failure))
    return fail("HTTP incipit exceeded max configured length");

  return succeed();
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
  str chunk_size;
  int chunk_size_int;

  do {
    result = socket_receive_until_str(
      &(http->sock), http_chunks.terminator, &chunk_size, HTTP_CHUNK_SIZE_MAXLEN);
    if (unlikely(result.failure))
      return fail("HTTP chunk size exceeded max configured length");

    result = str_to_int(chunk_size, &chunk_size_int);
    if (unlikely(result.failure))
      return fail("HTTP chunk size invalid");

    if (chunk_size_int == 0)
      break;
    
  } while (chunk_size_int > 0);
  /*

  FIGURE OUT HOW TO HANDLE CHUNK DATA ALLOCATION




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
  */
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
