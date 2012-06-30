#include "tst-secondary.h"

asm (".secondary bar");

int
bar (void)
{
  return SECONDARY;
}
