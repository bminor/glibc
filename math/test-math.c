#include <ansidecl.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int
DEFUN_VOID(main)
{
  CONST char str[] = "123.456";
  double x,y,z,h,li,lr,a,lrr;

  x = atof (str);
	
  printf ("%g %g\n", x, pow (10.0, 3.0));
  
  x = sinh(2.0);
  
  fprintf(stderr,"sinh(2.0) = %g\n", x);
  
  x = sinh(3.0);
  
  fprintf(stderr,"sinh(3.0) = %g\n", x);
  
  h = hypot(2.0,3.0);
  
  fprintf(stderr,"h=%g\n", h);
  
  a = atan2(3.0, 2.0);
  
  fprintf(stderr,"atan2(3,2) = %g\n", a);
  
  lr = pow(h,4.0);
  
  fprintf(stderr,"pow(%g,4.0) = %g\n", h, lr);
  
  lrr = lr;
  
  li = 4.0 * a;
  
  lr = lr / exp(a*5.0);
  
  fprintf(stderr,"%g / exp(%g * 5) = %g\n", lrr, exp(a*5.0), lr);
  
  lrr = li;
  
  li += 5.0 * log(h);
  
  fprintf(stderr,"%g + 5*log(%g) = %g\n", lrr, h, li);
  
  fprintf(stderr,"cos(%g) = %g,  sin(%g) = %g\n", li, cos(li), li, sin(li));
  
  fflush(stderr);
  
  return 0;
}
