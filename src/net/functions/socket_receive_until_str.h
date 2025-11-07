struct result socket_receive_until_str (
    struct socket* sock,
    str* pattern,
    uint max_length,
    struct allocator* allocator
);
