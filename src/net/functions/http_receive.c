static struct result http_receive_incipit (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  str incipit_inlined;
  uint allocator_initial_position = allocator_position_get(allocator);

  struct result result = socket_receive_until_str(&(http->socket),
    http_chunks.terminator, &incipit_inlined, HTTP_INCIPIT_MAXLEN, allocator);
  if (unlikely(result.failure))
    return fail("HTTP incipit exceeded max configured length");

  message->incipit.chars = allocator_push(allocator, incipit_inlined.length);
  message->incipit.length = incipit_inlined.length;
  memory_copy(message->incipit.chars, incipit_inlined.chars, incipit_inlined.length);
  allocator_position_set(allocator, allocator_initial_position);

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
  result = str_to_int(content_length, &content_length_number);
  if (unlikely(result.failure))
    return result;

  message->body.chars = allocator_push(allocator, content_length_number);
  message->body.length = content_length_number;

  result = socket_receive_all(&(http->socket),
    message->body.chars, message->body.length);
  if (unlikely(result.failure))
    return result;

  return succeed(); }

receive_body_chunked: {
  str chunk_size;
  int chunk_size_int;
  void* chunk_data_address;
  void* body_inlined_address;
  uint body_position = allocator_position_get(allocator);
  uint allocator_initial_position = allocator_position_get(allocator);

  do {
    result = socket_receive_until_str(&(http->socket),
      http_chunks.crlf, &chunk_size, HTTP_CHUNK_SIZE_MAXLEN, allocator);
    if (unlikely(result.failure))
      return fail("HTTP chunk size exceeded max configured length");

    /* Since HTTP designers have done an arguably bad job at thinking that these
      chunk sizes had to be represented in hexadecimal, we use the CRLF chars to
      move the digits two bytes forward and prepend a "0x" before the number, in
      order to be able to correctly parse the number as hexadecimal. [Reference:](
      https://datatracker.ietf.org/doc/html/rfc9112#name-chunked-transfer-coding)
      Example:
      |  1  4  1 CR LF | ascii  ->  |  0  x  1  4  1 | ascii
      | 31 34 31 0d 0a | bytes  ->  | 30 78 31 34 31 | bytes */
    memory_move(chunk_size.chars + http_chunks.crlf.length,
      chunk_size.chars, chunk_size.length);
    chunk_size.chars[0] = '0';
    chunk_size.chars[1] = 'x';

    result = str_to_int(&chunk_size, &chunk_size_int);
    if (unlikely(result.failure))
      return fail("HTTP chunk size invalid");

    allocator_position_set(allocator, body_position);

    if (chunk_size_int == 0)
      break;

    chunk_data_address = allocator_stretch(
      allocator, chunk_size_int + http_chunks.crlf.length);
    result = socket_receive_all(&(http->socket),
      chunk_data_address, chunk_size_int + http_chunks.crlf.length);
    if (unlikely(result.failure))
      return result;

    body_position += chunk_size_int;
    allocator_shrink(allocator, http_chunks.crlf.length);
    
  } while (chunk_size_int > 0);

  body_inlined_address = allocator_position_address(allocator, allocator_initial_position);
  message->body.length = allocator_position_get(allocator) - allocator_initial_position;
  message->body.chars = allocator_push(allocator, message->body.length);
  memory_copy(message->body.chars, body_inlined_address, message->body.length);
  allocator_shrink(allocator, message->body.length);

  return succeed(); }
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
