#include "tst-secondary.h"

int
__attribute__ ((weak))
bar (void)
{
  return WEAK;
}

void
foo (void)
{
}
