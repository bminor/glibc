#include <ansidecl.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int
DEFUN_VOID(main)
{
  CONST char str[] = "123.456";
  double x;

  x = atof (str);
	
  printf ("%g %g\n", x, pow (10.0, 3.0));

  return 0;
}
