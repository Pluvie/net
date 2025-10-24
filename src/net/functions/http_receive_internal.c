static inline str http_receive_incipit (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
}

static inline str http_receive_status (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  int buffer_length = 8192;
  char* buffer = allocator_push(allocator, buffer_length);
  memory_set(buffer, 0, buffer_length);

  socket_receive(&(http->socket), buffer_length - 1, buffer, 0);
  if (unlikely(failure.occurred))
    return (str) { 0 };

  str cursor = { buffer, buffer_length }; 

  str expected_incipit = string("HTTP/1.1 ");
  str incipit = { cursor.chars, expected_incipit.length };
  if (!str_equal(incipit, expected_incipit)) {
    fail("http_receive: unexpected response incipit: ", f(incipit));
    return cursor;
  }
  cursor.chars += incipit.length;

  str status_code = { cursor.chars, 4 };
  unsigned int status = str_to_int(&status_code);
  if (unlikely(failure.occurred)) {
    fail("http_receive: unexpected status code: ", f(status_code));
    return cursor;
  }
  if (unlikely(status < 100 || status > 599)) {
    fail("http_receive: unexpected status code: ", f(status));
    return cursor;
  }
  cursor.chars += status_code.length;
  message->status = status;

  str expected_crlf = string("\r\n");
  while (*cursor.chars != 0) {
    str crlf = { cursor.chars, expected_crlf.length };
    if (str_equal(crlf, expected_crlf))
      return (str) { cursor.chars, cursor.chars - buffer };
    cursor.chars++;
  }

  fail("http_receive: incipit too long");
  return cursor;
}
