void http_send (
    struct http* http,
    struct http_message* message
)
{
  /* Prepares the request inicipit: line and headers. Begins by creating the line
    section: `{METHOD} {PATH} HTTP/1.1\r\n`. */
  struct str_builder builder = str_builder(2048);
  str_build(&builder, message->method);
  str_build(&builder, string(" "));
  str_build(&builder, message->path);
  str_build(&builder, string(" HTTP/1.1"));
  str_build(&builder, string("\r\n"));

  /* Line section is terminated. Begins the headers section. Starts by adding the only
    mandatory HTTP header: Host. */
  str_build(&builder, string("Host: "));
  str_build(&builder, http->host);
  str_build(&builder, string("\r\n"));

  /* Adds user provided headers, if any. */
  for map_each(&(message->headers), str* name, str* value) {
    str_build(&builder, *name);
    str_build(&builder, string(": "));
    str_build(&builder, *value);
    str_build(&builder, string("\r\n"));
  }

  /* Adds the content length header, if a body is present. */
  if (!str_empty(message->body)) {
    char content_length[sizeof(INT_MAXNUM)] = { 0 };
    int number_length = prints(
      content_length, sizeof(content_length), f(message->body.length));
    str_build(&builder, string("Content-Length: "));
    str_build(&builder, (str) { content_length, number_length });
    str_build(&builder, string("\r\n"));
  }

  /* Terminates the headers section, and therefore the incipit. */
  str_build(&builder, string("\r\n"));

  /* Fires off the incipit. Hopefully no network errors occur. */
  socket_send(&(http->socket), builder.result.length, builder.result.chars, 0);
  if (unlikely(failure.occurred))
    return;

  /* Incipit is fired, free its required memory and then prepares to fire the body,
    if not empty. */
  memory_release(builder.memory);
  if (str_empty(message->body))
    return;

  /* HTTP protocol specifies that the message body is just a sequence of octets, so
    we are happy to fire it right now -- in its entirety. */
  socket_send(&(http->socket), message->body.length, message->body.chars, 0);
  if (unlikely(failure.occurred))
    return;
}
