#include <stdio.h>

int
bar (void)
{
  printf ("In %s:%s\n", __FILE__, __func__);
  return 123;
}
