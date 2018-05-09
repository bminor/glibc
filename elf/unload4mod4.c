#include <stdio.h>
#include <stdlib.h>

int
__attribute__((noinline))
/* Workaround for clang/lld failing to override this from a module.  */
__attribute__((weak))
baz (int x)
{
  abort ();
}

int
bar (int x)
{
  puts ("in bar");
  return baz (x + 1) + 2;
}
