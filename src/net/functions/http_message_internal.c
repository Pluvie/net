static enum result http_message_incipit_length (
    struct http_message* message,
    str host,
    uint* result
)
{
  uint length = 0;

  if (unlikely(message->method.length > HTTP_METHOD_MAXLEN))
    return fail("http method too long");

  if (unlikely(message->path.length > HTTP_PATH_MAXLEN))
    return fail("http path too long");

  /* Begins the line section. */
  length += message->method.length;
  length += http_chunks.space.length;
  length += message->path.length;
  length += http_chunks.space.length;
  length += http_chunks.version_1_1.length;
  length += http_chunks.crlf.length;

  /* Line section is terminated. Begins the headers section. Starts by adding the only
    mandatory HTTP header: Host. */
  length += sizeof("Host") - 1;
  length += http_chunks.colon.length;
  length += http_chunks.space.length;
  length += host.length;
  length += http_chunks.crlf.length;

  { /* Add the HTTP headers. */
    str* name; str* value; struct iterator iter = { 0 };
    while (http_headers_each(&(message->headers), &iter, &name, &value)) {
      if (unlikely(name->length > HTTP_HEADER_NAME_MAXLEN))
        return fail("http header name too long");

      if (unlikely(value->length > HTTP_HEADER_VALUE_MAXLEN))
        return fail("http header value too long");

      length += name->length;
      length += http_chunks.colon.length;
      length += http_chunks.space.length;
      length += value->length;
      length += http_chunks.crlf.length;
    }
  }

  /* Adds the content length header, if a body is present. */
  if (!str_empty(message->body)) {
    char buffer[INT_MAXCHARS] = { 0 };
    int buffer_length = sprintf(buffer, "%"fmt(UINT), message->body.length);
    length += sizeof("Content-Length") - 1;
    length += http_chunks.colon.length;
    length += http_chunks.space.length;
    length += buffer_length;
    length += http_chunks.crlf.length;
  }

  /* Terminates the headers section. */
  length += http_chunks.crlf.length;
  *result = length;

  return Success;
}

static enum result http_message_incipit_build (
    struct http_message* message,
    str host,
    struct allocator* allocator
)
{
  enum result result;
  str* incipit = &(message->incipit);
  uint incipit_length;
  char* incipit_begin;

  result = http_message_incipit_length(message, host, &incipit_length);
  if (unlikely(result == Failure))
    return Failure;

  incipit_begin = allocator_push(allocator, incipit_length);
  incipit->chars = incipit_begin;
  incipit->length = 0;

  /* Begins the line section. */
  str_append(incipit, message->method);
  str_append(incipit, http_chunks.space);
  str_append(incipit, message->path);
  str_append(incipit, http_chunks.space);
  str_append(incipit, http_chunks.version_1_1);
  str_append(incipit, http_chunks.crlf);

  { /* Line section is terminated. Begins the headers section. Starts by adding the only
      mandatory HTTP header: Host. */
    str host_header_name = str("Host");
    str_append(incipit, host_header_name);
    str_append(incipit, http_chunks.colon);
    str_append(incipit, http_chunks.space);
    str_append(incipit, host);
    str_append(incipit, http_chunks.crlf);
  }

  { /* Add the HTTP headers. */
    str* key; str* value; struct iterator iter = { 0 };
    while (http_headers_each(&(message->headers), &iter, &key, &value)) {
      str_append(incipit, *key);
      str_append(incipit, http_chunks.colon);
      str_append(incipit, http_chunks.space);
      str_append(incipit, *value);
      str_append(incipit, http_chunks.crlf);
    }
  }

  /* Adds the content length header, if a body is present. */
  if (!str_empty(message->body)) {
    str content_length_header_name = str("Content-Length");
    str content_length_header_value = { 0 };
    char buffer[INT_MAXCHARS] = { 0 };
    int buffer_length = sprintf(buffer, "%"fmt(UINT), message->body.length);
    content_length_header_value.chars = buffer;
    content_length_header_value.length = buffer_length;
    str_append(incipit, content_length_header_name);
    str_append(incipit, http_chunks.colon);
    str_append(incipit, http_chunks.space);
    str_append(incipit, content_length_header_value);
    str_append(incipit, http_chunks.crlf);
  }

  /* Terminates the headers section. */
  str_append(incipit, http_chunks.crlf);
  incipit->chars = incipit_begin;
  incipit->length = incipit_length;

  return Success;
}
