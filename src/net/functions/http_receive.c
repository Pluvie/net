bool str_starts (
    str source,
    str target
)
{
  return memory_equal(source.chars, target.chars, target.length);
}

int str_index (
    str source,
    str target
)
{
  int index = 0;

  do {
    if (target.length > source.length)
      return -1;

    if (str_starts(source, target))
      return index;

    if (source.length > 0) {
      source.chars++;
      source.length--;
      index++;
    }
  } while (source.length > 0);

  return -1;
}

bool str_contains (
    str source,
    str target
)
{
  return str_index(source, target) >= 0;
}

bool str_split (
    str source,
    str target,
    str* token,
    struct iterator* iter
)
{
  int index;

  if (!iter->initialized) {
    /* Unsigned integer overflow is well defined behaviour. When the first key of
      the map will be found, this index shall be incremented by 1, wrapping to 0.
      See: https://en.wikipedia.org/wiki/Integer_overflow */
    iter->index = (uint) -1;
    iter->initialized = true;
  }

  if (iter->position > source.length)
    return false;

  token->chars = source.chars + iter->position;
  token->length = source.length - iter->position;

  index = str_index(*token, target);
  if (index == -1)
    return false;

  token->length = index;
  iter->position += index + target.length;
  iter->index++;
  return true;
}

static struct result http_receive_incipit (
    struct http* http,
    struct allocator* allocator,
    str* incipit
)
{
  struct result result;
  str data, search;
  int index, bytes_to_receive;
  uint allocator_initial_position = allocator->line.position;

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
    bytes_to_receive = index - (search.length - data.length);
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
  str incipit = { 0 };

  result = http_receive_incipit(http, allocator, &incipit);
  if (unlikely(result.failure))
    return result;

  { str chunk = { 0 }; struct iterator iter = { 0 };
    while (str_split(incipit, http_chunks.crlf, &chunk, &iter)) {
      printl("Chunk: %"fmt(STR), str_fmt(chunk));
    }
  }
  printl("Incipit:");
  printl("%"fmt(STR), str_fmt(incipit));
  /*
  result = socket_parser_init(&parser, &(http->socket), allocator);
  if (unlikely(result.failure))
    return result;

  parser_result = socket_parser_until(&parser, http_chunks.space, HTTP_VERSION_MAXLEN);
  if (!str_equal(parser_result, http_chunks.version_1_1))
    return fail("expected HTTP version");
  printl("HTTP version: %"fmt(STR), str_fmt(parser_result));

  parser_result = socket_parser_until(&parser, http_chunks.space, HTTP_STATUS_MAXLEN);
  if (parser_result.length != 3)
    return fail("expected HTTP status code");

  result = str_to_int(&parser_result, (int*) &(message->status));
  if (unlikely(result.failure))
    return result;
  printl("HTTP status: %"fmt(INT), message->status);

  parser_result = socket_parser_until(&parser, http_chunks.crlf, HTTP_REASON_MAXLEN);
  if (parser_result.length == 0)
    return fail("expected CRLF after HTTP status description");

  message->headers = http_headers_alloc(16, allocator);

parse_header:
  parser_result = socket_parser_until(&parser, http_chunks.colon, HTTP_HEADER_NAME_MAXLEN);
  if (parser_result.length == 0)
    return fail("expected colon after HTTP header name");
  header.name = parser_result;

  parser_result = socket_parser_until(&parser, http_chunks.crlf, HTTP_HEADER_VALUE_MAXLEN);
  if (parser_result.length == 0)
    return fail("expected CRLF after HTTP header value");
  header.value = parser_result;
  http_headers_set(&(message->headers), header.name, header.value);*/

  return succeed();
}
