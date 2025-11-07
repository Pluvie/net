struct result socket_receive_until_str (
    struct socket* sock,
    str terminator,
    str* output,
    uint max_length,
    struct allocator* allocator
)
{
  struct result result;
  str data, search;
  int index, bytes_to_receive;
  uint allocator_initial_position = allocator_position_get(allocator);
  uint chunk_size = 32 * terminator.length;

  output->length = 0;

receive_chunk:
  if (output->length > max_length)
    goto max_length_overflow;

  data.chars = allocator_stretch(allocator, chunk_size);
  data.length = chunk_size;

  result = socket_peek(sock, data.chars, chunk_size, (int*) &(data.length));
  if (unlikely(result.failure))
    return result;

  if (output->length > 0 && data.length >= terminator.length) {
    search.chars = data.chars - terminator.length;
    search.length = data.length + terminator.length;
  } else {
    search.chars = data.chars;
    search.length = data.length;
  }

  index = str_index(search, terminator);

  if (index >= 0) {
    bytes_to_receive = index - (search.length - data.length) + terminator.length;
    result = socket_receive(sock, data.chars, bytes_to_receive, nullptr);
    if (unlikely(result.failure))
      return result;

    output->length += bytes_to_receive;
    output->chars = allocator_position_address(allocator, allocator_initial_position);
    return succeed();
  }

  result = socket_receive(sock, data.chars, chunk_size, nullptr);
  if (unlikely(result.failure))
    return result;

  output->length += chunk_size;
  goto receive_chunk;

max_length_overflow:
  return fail("max length overflow");
}
