#ifndef	_MATH_H

#ifdef _ISOMAC
# undef NO_LONG_DOUBLE
#endif

#include <math/math.h>

#ifndef _ISOMAC
# include <libm-symbols.h>

/* Now define the internal interfaces.  */
extern int __signgam;

libc_libm_hidden_mathcall (finite)
libc_libm_hidden_mathcall (isinf)
libc_libm_hidden_mathcall (isnan)
libc_libm_hidden_mathcall (scalbn)
libc_libm_hidden_mathcall (signbit)

libm_hidden_mathcall (atan)
libm_hidden_mathcall (atan2)
libm_hidden_mathcall (ceil)
libm_hidden_mathcall (cos)
libm_hidden_mathcall (erf)
libm_hidden_mathcall (erfc)
libm_hidden_mathcall (exp)
libm_hidden_mathcall (expm1)
libm_hidden_mathcall (floor)
libm_hidden_mathcall (fpclassify)
libm_hidden_mathcall (frexp)
libm_hidden_mathcall (hypot)
libm_hidden_mathcall (ilogb)
libm_hidden_mathcall (issignaling)
libm_hidden_mathcall (ldexp)
libm_hidden_mathcall (log)
libm_hidden_mathcall (log1p)
libm_hidden_mathcall (nearbyint)
libm_hidden_mathcall (nextup)
libm_hidden_mathcall (rint)
libm_hidden_mathcall (round)
libm_hidden_mathcall (roundeven)
libm_hidden_mathcall (scalbln)
libm_hidden_mathcall (sin)
libm_hidden_mathcall (sincos)
libm_hidden_mathcall (sqrt)
libm_hidden_mathcall (trunc)

# if !(defined __FINITE_MATH_ONLY__ && __FINITE_MATH_ONLY__ > 0)
#  ifndef NO_MATH_REDIRECT
/* Declare sqrt for use within GLIBC.  Compilers typically inline sqrt as a
   single instruction.  Use an asm to avoid use of PLTs if it doesn't.  */
float (sqrtf) (float) asm ("__ieee754_sqrtf") libm_attr_hidden;
double (sqrt) (double) asm ("__ieee754_sqrt") libm_attr_hidden;
#   ifndef __NO_LONG_DOUBLE_MATH
long double (sqrtl) (long double) asm ("__ieee754_sqrtl") libm_attr_hidden;
#   endif
#   if __HAVE_DISTINCT_FLOAT128 > 0
_Float128 (sqrtf128) (_Float128) asm ("__ieee754_sqrtf128") libm_attr_hidden;
#   endif
#  endif
# endif

#endif
#endif
