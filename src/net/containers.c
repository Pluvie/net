#define container_for_key str
#define container_for_value str
#define container_name http_headers
#define container_equalizer(a, b) str_equal_case(a, b)
#define container_hasher(e) hash_djb2(e.chars, e.length)
#include <ion/containers/map.c>
