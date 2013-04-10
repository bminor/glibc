#include <math.h>

/* The PPC32 default implementation will fallback to __ieee754_sqrt symbol
   from sysdeps/powerpc/fpu/e_sqrt.c  */

#define __sqrt __sqrt_ppc32
#undef weak_alias
#define weak_alias(a, b)
#undef strong_alias
#define strong_alias(a, b)


#include <math/w_sqrt.c>
