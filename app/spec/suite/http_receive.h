#include "../spec.h"

static struct {
  str message;
  unsigned int cursor;
  bool timeout;
} sim;

static int sim_socket_receive (
    struct socket* sock,
    unsigned int length,
    void* buffer,
    int flags
)
{
  if (sim.cursor + length <= sim.message.length) {
    memory_copy(buffer, sim.message.chars + sim.cursor, length);
    sim.cursor += length;
    return length;
  }

  unsigned int remainder = sim.message.length - sim.cursor;
  if (remainder > 0) {
    memory_copy(buffer, sim.message.chars + sim.cursor, remainder);
    sim.cursor += remainder;
    return remainder;
  }

  sim.timeout = true;
  return 0;
}

#define socket_receive(...) sim_socket_receive(__VA_ARGS__)

/*
  Creates a spec-specific copy of the function so that the linker does not find
  multiple definitions of this function with the same name.
*/
#define http_receive  specced__http_receive
#include <net/functions/http_receive.c>
