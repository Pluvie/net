#include "spec.h"
#include "spec.c"

spec( http_receive );

int0 main (
    int0 argc,
    char** argv
)
{
  spec_focus( http_receive );

  specs_run();

  if (specs_passed)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
