#include "http_receive_internal.c"

struct socket_parser {
  struct socket* socket;
  struct allocator* allocator;
  str data;
  char* cursor;
  uint position;
  uint chunk_size;
};

enum result socket_parser_init (
    struct socket_parser* parser,
    struct socket* sock,
    struct allocator* allocator
)
{
  parser->socket = sock;
  parser->allocator = allocator;

  if (parser->chunk_size < SOCKET_PARSER_CHUNK_MINSIZE)
    parser->chunk_size = SOCKET_PARSER_CHUNK_MINSIZE;

  parser->data.chars = allocator_stretch(parser->allocator, parser->chunk_size);
  parser->cursor = parser->data.chars;
  parser->position = 0;

  return socket_receive(parser->socket,
    parser->data.chars,
    parser->chunk_size,
    (int*) &(parser->data.length));
}

enum result socket_parser_advance (
    struct socket_parser* parser,
    uint amount
)
{
  enum result result;
  uint amount_to_extend;
  int received_bytes;

  if (parser->position + amount < parser->data.length) {
    parser->cursor += amount;
    parser->position += amount;
    return Success;
  }

  amount_to_extend = parser->chunk_size;
  if (amount > amount_to_extend)
    amount_to_extend = amount;

  parser->data.chars = allocator_stretch(parser->allocator, amount_to_extend);
  parser->position += amount;
  parser->cursor = parser->data.chars + parser->position;

  result = socket_receive(parser->socket, parser->cursor, amount_to_extend, &received_bytes);
  if (unlikely(result == Failure))
    return Failure;

  parser->data.length += received_bytes;
  return Success;
}

str socket_parser_until (
    struct socket_parser* parser,
    str value
)
{
  str result = { 0 };
  str look_ahead = { 0 };
  char* begin_cursor = parser->cursor;
  uint begin_position = parser->position;

  while (parser->position < parser->data.length) {
    look_ahead.chars = parser->cursor;
    look_ahead.length = value.length;
    if ((parser->data.length - parser->position) < look_ahead.length)
      return result;

    if (str_equal(look_ahead, value)) {
      result.chars = begin_cursor;
      result.length = parser->position - begin_position;
      socket_parser_advance(parser, 1);
      return result;
    }

    socket_parser_advance(parser, 1);
  }
  return result;
}


enum result http_receive (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  enum result result;
  struct http_header header = { 0 };
  struct socket_parser parser = { 0 };
  str parser_result = { 0 };

  result = socket_parser_init(&parser, &(http->socket), allocator);
  if (unlikely(result == Failure))
    return Failure;

  parser_result = socket_parser_until(&parser, http_chunks.space);
  if (!str_equal(parser_result, http_chunks.version_1_1))
    return fail("expected HTTP version");
  printl("HTTP version: %"fmt(STR), str_fmt(parser_result));

  parser_result = socket_parser_until(&parser, http_chunks.space);
  if (parser_result.length != 3)
    return fail("expected HTTP status code");

  result = str_to_int(&parser_result, (int*) &(message->status));
  if (unlikely(result == Failure))
    return Failure;
  printl("HTTP status: %"fmt(INT), message->status);

  parser_result = socket_parser_until(&parser, http_chunks.crlf);
  if (parser_result.length == 0)
    return fail("expected CRLF after HTTP status description");

  /* Parse headers. */
  message->headers = http_headers_alloc(16, allocator);

/*parser_header:*/
  parser_result = socket_parser_until(&parser, http_chunks.colon);
  if (parser_result.length == 0)
    return fail("expected colon after HTTP header name");
  header.name = parser_result;

  parser_result = socket_parser_until(&parser, http_chunks.crlf);
  if (parser_result.length == 0)
    return fail("expected CRLF after HTTP header value");
  header.value = parser_result;
  http_headers_set(&(message->headers), header.name, header.value);

  /*
  printl("Received:");
  printl("%"fmt(CSTR), buffer);
  */

  return Success;
}
