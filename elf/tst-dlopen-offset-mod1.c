#include <stdio.h>

int
foo (void)
{
  printf ("In %s:%s\n", __FILE__, __func__);
  return 1;
}

int
foosub (void)
{
  printf ("In %s:%s\n", __FILE__, __func__);
  return 20;
}
