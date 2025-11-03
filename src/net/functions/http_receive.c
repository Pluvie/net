#include "http_receive_internal.c"

struct str_parser {
  str data;
  char* cursor;
  uint position;
};

void str_parse_begin (
    struct str_parser* parser,
    char* content,
    uint length
)
{
  parser->data.chars = content;
  parser->data.length = length;
  parser->cursor = content;
  parser->position = 0;
}

void str_parse_advance (
    struct str_parser* parser,
    uint amount
)
{
  if (parser->position + amount > parser->data.length) {
    parser->position = parser->data.length;
  } else {
    parser->cursor += amount;
    parser->position += amount;
  }
}

str str_parse_until (
    struct str_parser* parser,
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
      str_parse_advance(parser, 1);
      return result;
    }

    str_parse_advance(parser, 1);
  }
  return result;
}

int http_receive (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  cstr buffer = allocator_push(allocator, HTTP_BUFFER_LENGTH);
  str parse_result = { 0 };
  int received_bytes;
  enum result result;
  struct str_parser parser = { 0 };

  result = socket_receive(&(http->socket), buffer, HTTP_BUFFER_LENGTH, &received_bytes);
  if (unlikely(result == Failure))
    return Failure;

  str_parse_begin(&parser, buffer, received_bytes);

  parse_result = str_parse_until(&parser, http_chunks.space);
  if (!str_equal(parse_result, http_chunks.version_1_1))
    return fail("expected HTTP version");
  printl("HTTP version: %"fmt(STR), str_fmt(parse_result));

  parse_result = str_parse_until(&parser, http_chunks.space);
  if (parse_result.length != 3)
    return fail("expected HTTP status code");
  printl("HTTP status: %"fmt(STR), str_fmt(parse_result));

  result = str_to_int(&parse_result, (int*) &(message->status));
  if (unlikely(result == Failure))
    return Failure;

  /*
  printl("Received:");
  printl("%"fmt(CSTR), buffer);
  */

  return Success;
}
