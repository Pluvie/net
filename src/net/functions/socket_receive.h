Attribute__No_Discard
struct result socket_receive (
    struct socket* sock,
    void* data,
    uint length,
    int* received_bytes_ptr
);

Attribute__No_Discard
struct result socket_receive_str (
    struct socket* sock,
    str* data
);
