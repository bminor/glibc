#include <ansidecl.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int
DEFUN_VOID(main)
{
  CONST char str[] = "123.456";
  double x,h,li,lr,a,lrr;

  x = atof (str);
	
  printf ("%g %g\n", x, pow (10.0, 3.0));
  
  x = sinh(2.0);
  
  printf("sinh(2.0) = %g\n", x);
  
  x = sinh(3.0);
  
  printf("sinh(3.0) = %g\n", x);
  
  h = hypot(2.0,3.0);
  
  printf("h=%g\n", h);
  
  a = atan2(3.0, 2.0);
  
  printf("atan2(3,2) = %g\n", a);
  
  lr = pow(h,4.0);
  
  printf("pow(%g,4.0) = %g\n", h, lr);
  
  lrr = lr;
  
  li = 4.0 * a;
  
  lr = lr / exp(a*5.0);
  
  printf("%g / exp(%g * 5) = %g\n", lrr, exp(a*5.0), lr);
  
  lrr = li;
  
  li += 5.0 * log(h);
  
  printf("%g + 5*log(%g) = %g\n", lrr, h, li);
  
  printf("cos(%g) = %g,  sin(%g) = %g\n", li, cos(li), li, sin(li));
  
  x = drem(10.3435,6.2831852);

  printf("drem(10.3435,6.2831852) = %g\n", x);

  x = drem(-10.3435,6.2831852);

  printf("drem(-10.3435,6.2831852) = %g\n", x);

  x = drem(-10.3435,-6.2831852);
	
  printf("drem(-10.3435,-6.2831852) = %g\n", x);

  x = drem(10.3435,-6.2831852);

  printf("drem(10.3435,-6.2831852) = %g\n", x);


  printf("x%8.6gx\n", .5);
  printf("x%-8.6gx\n", .5);
  printf("x%6.6gx\n", .5);

  puts (atof ("-1e-17-") == -1e-17 ? "4 Worked!" : "4 Failed!");

  return 0;
}
