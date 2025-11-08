struct result socket_receive_until_str (
    struct socket* sock,
    str terminator,
    str* output,
    uint max_length,
    struct buffer* buffer
);
