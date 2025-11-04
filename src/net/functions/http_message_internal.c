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

  if (index > 0)
    iter->index++;

  token->length = index;
  iter->position += index + target.length;
  return true;
}

static struct result http_message_incipit_length (
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

  return succeed();
}

static struct result http_message_incipit_build (
    struct http_message* message,
    str host,
    struct allocator* allocator
)
{
  struct result result;
  str* incipit = &(message->incipit);
  uint incipit_length;
  char* incipit_begin;

  result = http_message_incipit_length(message, host, &incipit_length);
  if (unlikely(result.failure))
    return result;

  incipit_begin = allocator_push(allocator, incipit_length);
  incipit->chars = incipit_begin;
  incipit->length = incipit_length;

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

  return succeed();
}

static struct result http_message_incipit_decode (
    struct http_message* message,
    struct allocator* allocator
)
{
  str line;
  line.chars = message->incipit.chars;
  line.length = str_index(message->incipit, http_chunks.crlf);
  line = str_strip(line);

  { str chunk = { 0 }; struct iterator iter = { 0 };
    while (str_split(message->incipit, http_chunks.space, &chunk, &iter)) {
      if (chunk.length == 0) continue;
      switch (iter.index) {
      case 0:
        message->protocol = chunk;
        printl("Protocol: |%"fmt(STR)"|", str_fmt(chunk));
        break;
      case 1:
        /*message->status = chunk;*/
        printl("Status: |%"fmt(STR)"|", str_fmt(chunk));
        break;
      }
    }
  }

  return succeed();
}
