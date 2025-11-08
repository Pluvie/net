static struct result http_receive_incipit (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  str incipit;
  struct buffer buffer = buffer_init(0);

  struct result result = socket_receive_until_str(&(http->socket),
    Http.Terminator, &incipit, HTTP_INCIPIT_MAXLEN, &buffer);
  if (unlikely(result.failure))
    return fail("HTTP incipit exceeded max configured length");

  message->incipit.chars = allocator_push(allocator, incipit.length);
  message->incipit.length = incipit.length;
  memory_copy(message->incipit.chars, incipit.chars, incipit.length);
  buffer_release(&buffer);

  return succeed();
}

static struct result http_receive_body (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  struct result result;

  if (http_headers_has(&(message->headers), Http.Content_Length))
    goto receive_body_entirely;

  if (http_headers_has(&(message->headers), Http.Transfer_Encoding)) {
    str* transfer_encoding = http_headers_get(&(message->headers), Http.Transfer_Encoding);
    if (str_contains(*transfer_encoding, Http.Chunked_Transfer))
      goto receive_body_chunked;
  }

  /* No body to receive. */
  return succeed();

receive_body_entirely: {
  str* content_length;
  int content_length_number;

  content_length = http_headers_get(&(message->headers), Http.Content_Length);
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
  uint body_buffer_position = 0;
  struct buffer buffer = buffer_init(0);

  do {
    result = socket_receive_until_str(&(http->socket),
      Http.Crlf, &chunk_size, HTTP_CHUNK_SIZE_MAXLEN, &buffer);
    if (unlikely(result.failure))
      return fail("HTTP chunk size exceeded max configured length");

    /* Since HTTP designers have done an arguably bad job at thinking that these
      chunk sizes had to be represented in hexadecimal, we use the CRLF chars to
      move the digits two bytes forward and prepend a "0x" before the number, in
      order to be able to correctly parse the number as hexadecimal. [Reference:](
      https://datatracker.ietf.org/doc/html/rfc9112#name-chunked-transfer-coding)
      Example:
      |  1  4  1 \r \n | ascii  ->  |  0  x  1  4  1 | ascii
      | 31 34 31 0d 0a | bytes  ->  | 30 78 31 34 31 | bytes */
    memory_move(chunk_size.chars + Http.Crlf.length, chunk_size.chars, chunk_size.length);
    chunk_size.chars[0] = '0';
    chunk_size.chars[1] = 'x';

    result = str_to_int(&chunk_size, &chunk_size_int);
    if (unlikely(result.failure))
      return fail("HTTP chunk size invalid");

    buffer_position_set(&buffer, body_buffer_position);
    if (chunk_size_int == 0)
      break;

    chunk_data_address = buffer_push(&buffer, chunk_size_int + Http.Crlf.length);
    result = socket_receive_all(&(http->socket),
      chunk_data_address, chunk_size_int + Http.Crlf.length);
    if (unlikely(result.failure))
      return result;

    body_buffer_position += chunk_size_int;
    buffer_pop(&buffer, Http.Crlf.length);
    
  } while (chunk_size_int > 0);

  message->body.length = buffer_position_get(&buffer);
  message->body.chars = allocator_push(allocator, message->body.length);
  memory_copy(message->body.chars, buffer_address_at(&buffer, 0), message->body.length);
  buffer_release(&buffer);

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
