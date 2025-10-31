struct {
  str space;
  str colon;
  str crlf;
  str version_1;
  str version_1_1;
  str version_2;
  str version_3;
} http_chunks = {
  str(" "),
  str(":"),
  str("\r\n"),
  str("HTTP/1"),
  str("HTTP/1.1"),
  str("HTTP/2"),
  str("HTTP/3"),
};


struct str_builder {
  char* memory;
  uint length;
  uint capacity;
};

void str_build (
    struct str_builder* builder,
    str string
)
{
  builder->memory = memory_resize(builder->memory, builder->length + string.length);
  memory_copy(builder->memory + builder->length, string.chars, string.length);
  builder->length += string.length;
}


int http_send (
    struct http* http,
    struct http_message* message,
    struct allocator* allocator
)
{
  int error;
  str body = message->body.content;
  struct str_builder builder = { 0 };

  if (unlikely(message->method.length > HTTP_MAX_METHODLEN))
    return fail("http method too long");
  if (unlikely(message->path.length > HTTP_MAX_PATHLEN))
    return fail("http path too long");

  /* Begins the line section. */
  str_build(&builder, message->method);
  str_build(&builder, http_chunks.space);
  str_build(&builder, message->path);
  str_build(&builder, http_chunks.space);
  str_build(&builder, http_chunks.version_1_1);
  str_build(&builder, http_chunks.crlf);

  { /* Line section is terminated. Begins the headers section. Starts by adding the only
      mandatory HTTP header: Host. */
    struct http_header host_header = { str("Host: "), { 0 } };
    memory_copy(&(host_header.value), &(http->host), sizeof(http->host));
    str_build(&builder, host_header.key);
    str_build(&builder, http_chunks.space);
    str_build(&builder, host_header.value);
    str_build(&builder, http_chunks.crlf);
  }

  { /* Add the HTTP headers. */
    str* key; str* value; struct iterator iter = { 0 };
    while (http_headers_each(&(message->headers), &iter, &key, &value)) {
      str_build(&builder, *key);
      str_build(&builder, http_chunks.colon);
      str_build(&builder, http_chunks.space);
      str_build(&builder, *value);
      str_build(&builder, http_chunks.crlf);
    }
  }

  /* Adds the content length header, if a body is present. */
  if (!str_empty(body)) {
    struct http_header content_length = { str("Content-Length: "), { 0 } };
    char buffer[INT_MAXCHARS] = { 0 };
    int length = sprintf(buffer, "%"fmt(UINT), body.length);
    content_length.value.chars = buffer;
    content_length.value.length = length;
    str_build(&builder, content_length.key);
    str_build(&builder, content_length.value);
    str_build(&builder, http_chunks.crlf);
  }

  /* Terminates the headers section. */
  str_build(&builder, http_chunks.crlf);
  printl("---------------------");
  printl("%"fmt(STR), (int_t) builder.length, builder.memory);
  printl("---------------------");

  error = socket_send(&(http->socket), builder.length, builder.memory, 0);
  memory_release(builder.memory);
  if (unlikely(error == -1))
    return FAILURE;

  if (str_empty(body))
    return SUCCESS;

  /* HTTP protocol specifies that the message body is just a sequence of octets, so
    we are happy to fire it right now -- in its entirety. */
  error = socket_send(&(http->socket), body.length, body.chars, 0);
  if (unlikely(error == -1))
    return FAILURE;

  return SUCCESS;
}
