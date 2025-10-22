#include "spec.h"
#include "spec.c"

spec( json_decode );

int0 main (
    int0 argc,
    char** argv
)
{
  spec_add( json_decode );

  specs_run();

  if (specs_passed)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
