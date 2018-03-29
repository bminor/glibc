#include <stdio.h>

int
xyzzy (void)
{
  printf ("In %s:%s\n", __FILE__, __func__);
  return 21;
}
