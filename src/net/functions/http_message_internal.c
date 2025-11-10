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

str str_sub (
    str source,
    int index_start,
    int index_end
)
{
  str result = { 0 };
  char* end_ptr;

  if (unlikely(index_start < 0))
    index_start = 0;

  if (index_end < 0)
    index_end = source.length + (index_end + 1);

  if (index_start >= index_end)
    return result;

  end_ptr = source.chars + index_end;
  result.chars = source.chars + index_start;
  result.length = end_ptr - source.chars - index_start;
  return result;
}

int str_partition (
    str source,
    str separator,
    str* left,
    str* right
)
{
  int index = str_index(source, separator);
  if (index == -1)
    return index;

  left->chars = source.chars;
  left->length = index;

  right->chars = source.chars + index + separator.length;
  right->length = source.length - index - separator.length;

  return index;
}

static struct result http_message_incipit_encode (
    struct http_message* message,
    str host,
    struct allocator* allocator
)
{
  str* incipit = &(message->incipit);
  uint incipit_length = 0;
  char* incipit_begin;
  char content_length_buffer[INT_MAXNUM] = { 0 };
  int content_length_buffer_size;

  if (unlikely(message->method.length > HTTP_METHOD_MAXLEN))
    return fail("http method too long");

  if (unlikely(message->path.length > HTTP_PATH_MAXLEN))
    return fail("http path too long");

  /* Begins the line section. */
  incipit_length += message->method.length;
  incipit_length += Http.Space.length;
  incipit_length += message->path.length;
  incipit_length += Http.Space.length;
  incipit_length += Http.Version_1_1.length;
  incipit_length += Http.Crlf.length;

  /* Line section is terminated. Begins the headers section. Starts by adding the only
    mandatory HTTP header: Host. */
  incipit_length += Http.Host.length;
  incipit_length += Http.Colon.length;
  incipit_length += Http.Space.length;
  incipit_length += host.length;
  incipit_length += Http.Crlf.length;

  { /* Add the HTTP headers. */
    str* name; str* value; struct iterator iter = { 0 };
    while (http_headers_each(&(message->headers), &iter, &name, &value)) {
      if (unlikely(name->length > HTTP_HEADER_NAME_MAXLEN))
        return fail("http header name too long");

      if (unlikely(value->length > HTTP_HEADER_VALUE_MAXLEN))
        return fail("http header value too long");

      incipit_length += name->length;
      incipit_length += Http.Colon.length;
      incipit_length += Http.Space.length;
      incipit_length += value->length;
      incipit_length += Http.Crlf.length;
    }
  }

  /* Adds the content length header, if a body is present. */
  if (!str_empty(message->body)) {
    content_length_buffer_size = sprintf(
      content_length_buffer, "%"fmt(UINT), message->body.length);
    incipit_length += Http.Content_Length.length;
    incipit_length += Http.Colon.length;
    incipit_length += Http.Space.length;
    incipit_length += content_length_buffer_size;
    incipit_length += Http.Crlf.length;
  }

  /* Terminates the headers section. */
  incipit_length += Http.Crlf.length;

  incipit_begin = allocator_push(allocator, incipit_length);
  incipit->chars = incipit_begin;
  incipit->length = incipit_length;

  /* Begins the line section. */
  str_append(incipit, message->method);
  str_append(incipit, Http.Space);
  str_append(incipit, message->path);
  str_append(incipit, Http.Space);
  str_append(incipit, Http.Version_1_1);
  str_append(incipit, Http.Crlf);

  /* Line section is terminated. Begins the headers section. Starts by adding the only
    mandatory HTTP header: Host, unless already specified by the user defined headers. */
  if (!http_headers_has(&(message->headers), Http.Host)) {
    str_append(incipit, Http.Host);
    str_append(incipit, Http.Colon);
    str_append(incipit, Http.Space);
    str_append(incipit, host);
    str_append(incipit, Http.Crlf);
  }

  { /* Add the HTTP headers. */
    str* key; str* value; struct iterator iter = { 0 };
    while (http_headers_each(&(message->headers), &iter, &key, &value)) {
      str_append(incipit, *key);
      str_append(incipit, Http.Colon);
      str_append(incipit, Http.Space);
      str_append(incipit, *value);
      str_append(incipit, Http.Crlf);
    }
  }

  /* Adds the content length header, if a body is present. */
  if (!str_empty(message->body)) {
    str content_length = { 0 };
    content_length.chars = content_length_buffer;
    content_length.length = content_length_buffer_size;
    str_append(incipit, Http.Content_Length);
    str_append(incipit, Http.Colon);
    str_append(incipit, Http.Space);
    str_append(incipit, content_length);
    str_append(incipit, Http.Crlf);
  }

  /* Terminates the headers section. */
  str_append(incipit, Http.Crlf);
  incipit->chars = incipit_begin;
  incipit->length = incipit_length;

  return succeed();
}

static struct result http_message_incipit_decode (
    struct http_message* message,
    struct allocator* allocator
)
{
  struct result result;
  str line;
  str headers;

  line.chars = message->incipit.chars;
  line.length = str_index(message->incipit, Http.Crlf);
  line = str_strip(line);

  { /* Splitting the line section (e.g. `HTTP/1.1 200 OK`) by space, which separates the
      version (`HTTP/1.1`), the status (`200`), and the reason phrase (`OK`). */
    str chunk = { 0 }; struct iterator iter = { 0 };
    while (str_split(line, Http.Space, &chunk, &iter)) {
      if (chunk.length == 0) continue;
      switch (iter.index) {
      case 0:
        /* Protocol version. */
        message->protocol = chunk;
        break;
      case 1:
        /* Status code. */
        result = str_to_int(&chunk, (int*) &(message->status));
        if (unlikely(result.failure))
          return result;
        break;
      default:
        /* All other values are discarded, including the reason phrase. This is
          recommended by the [HTTP RFC 7230 section 3.1.2](
          https://datatracker.ietf.org/doc/html/rfc7230#section-3.1.2):

          > The reason-phrase element exists for the sole purpose of providing a
          > textual description associated with the numeric status code, mostly
          > out of deference to earlier Internet application protocols that were
          > more frequently used with interactive text clients. A client SHOULD
          > ignore the reason-phrase content. */
        break;
      }
    }
  }

  headers = str_sub(message->incipit, line.length + Http.Crlf.length, -1);
  message->headers = http_headers_alloc(16, allocator);

  { /* Splitting the headers section (e.g. `Content-Type: text/plain`) by crlf, which
      separates each header line.. */
    str header = { 0 }; str name = { 0 }; str value = { 0 };
    int index; struct iterator iter = { 0 };

    while (str_split(headers, Http.Crlf, &header, &iter)) {
      if (header.length == 0) continue;

      index = str_partition(header, Http.Colon, &name, &value);
      if (unlikely(index == -1))
        return fail("invalid header");

      http_headers_set(&(message->headers), str_strip(name), str_strip(value));
    }
  }

  return succeed();
}
