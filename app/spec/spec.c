#include "spec.h"
#include "spec.c"

spec( http_receive );

int_t main (
    int_t argc,
    cstr* argv
)
{
  spec_focus( http_receive );

  specs_run();

  if (specs_passed)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
