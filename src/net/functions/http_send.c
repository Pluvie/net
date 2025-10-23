void http_send (
    struct http* http,
    struct http_message* message
)
{
  /* Prepares the request inicipit: line and headers. Begins by creating the line
    section: `{METHOD} {PATH} HTTP/1.1\r\n`. */
  struct str_builder incipit = str_builder(1024);
  str_builder_add(&incipit, message->method);
  str_builder_add(&incipit, string(" "));
  str_builder_add(&incipit, message->path);
  str_builder_add(&incipit, string(" HTTP/1.1"));
  str_builder_add(&incipit, string("\r\n"));

  /* Line section is terminated. Begins the headers section. Starts by adding the only
    mandatory HTTP header: Host. */
  str_builder_add(&incipit, string("Host: "));
  str_builder_add(&incipit, http->host);
  str_builder_add(&incipit, string("\r\n"));

  /* Adds user provided headers, if any. */
  for map_each(&(message->headers), str* name, str* value) {
    str_builder_add(&incipit, *name);
    str_builder_add(&incipit, string(": "));
    str_builder_add(&incipit, *value);
    str_builder_add(&incipit, string("\r\n"));
  }

  /* Adds the content length header, if a body is present. */
  if (!str_empty(message->body)) {
    char content_length[sizeof(INT_MAXNUM)] = { 0 };
    int number_length = prints(
      content_length, sizeof(content_length), f(message->body.length));
    str_builder_add(&incipit, string("Content-Length: "));
    str_builder_add(&incipit, (str) { content_length, number_length });
    str_builder_add(&incipit, string("\r\n"));
  }

  /* Terminates the headers section, and therefore the incipit. */
  str_builder_add(&incipit, string("\r\n"));

  /* Fires off the incipit. Hopefully no network errors occur. */
  socket_send(&(http->socket), incipit.result.length, incipit.result.chars);
  if (unlikely(failure.occurred))
    return;

  /* Incipit is fired, free its required memory and then prepares to fire the body,
    if not empty. */
  memory_release(incipit.memory);
  if (str_empty(message->body))
    return;

  /* HTTP protocol specifies that the message body is just a sequence of octets, so
    we are happy to fire it right now -- in its entirety. */
  socket_send(&(http->socket), message->body.length, message->body.chars);
  if (unlikely(failure.occurred))
    return;
}
