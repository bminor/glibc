#include <ansidecl.h>
#include <stdio.h>

int
main (void)
{
  int i;
  for (i = 0; i < 40; i++)
    printf ("%s = %d\n", "wow", i);
  return 0;
}
