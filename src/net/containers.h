#define set_of str
#include <ion/containers/set.h>

#define map_of str, str
#include <ion/containers/map.h>

#if standard(>= C11)
#define map_function(type, func, ...)       \
  _Generic(type,                            \
    map<str, str> : map<str, str>_ ## func  \
  )
#endif
