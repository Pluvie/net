#define set_of str
#define set_cmp_function(v, u) str_equal(v, u)
#define set_hash_function(v) hash_djb2(v.chars, v.length)
#include <ion/containers/set.c>

#define map_of str, str
#include <ion/containers/map.c>
