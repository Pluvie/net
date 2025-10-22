#include "spec.h"
#include "containers.h"

/*
  Lists.
*/

#define list_of int
#define list_cmp_function(v, u) v == u
#include <ion/containers/list.c>

/*
  Sets.
*/

#define set_of str
#define set_cmp_function(v, u) str_equal(v, u)
#define set_hash_function(v) hash_djb2(v.chars, v.length)
#include <ion/containers/set.c>

/*
  Maps.
*/

#define map_of str, struct squadmate
#include <ion/containers/map.c>
