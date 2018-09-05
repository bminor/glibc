#ifndef	_MATH_H

#ifdef _ISOMAC
# undef NO_LONG_DOUBLE
#endif

#include <math/math.h>

#ifndef _ISOMAC
/* Now define the internal interfaces.  */
extern int __signgam;

# if IS_IN (libc) || IS_IN (libm)
hidden_proto (__finite)
hidden_proto (__isinf)
hidden_proto (__isnan)
hidden_proto (__finitef)
hidden_proto (__isinff)
hidden_proto (__isnanf)

#  ifndef __NO_LONG_DOUBLE_MATH
hidden_proto (__finitel)
hidden_proto (__isinfl)
hidden_proto (__isnanl)
#  endif

#  if __HAVE_DISTINCT_FLOAT128
hidden_proto (__finitef128)
hidden_proto (__isinff128)
hidden_proto (__isnanf128)
hidden_proto (__signbitf128)
#  endif
# endif

libm_hidden_proto (__fpclassify)
libm_hidden_proto (__fpclassifyf)
libm_hidden_proto (__issignaling)
libm_hidden_proto (__issignalingf)
libm_hidden_proto (__exp)
libm_hidden_proto (__expf)
libm_hidden_proto (__roundeven)

# ifndef __NO_LONG_DOUBLE_MATH
libm_hidden_proto (__fpclassifyl)
libm_hidden_proto (__issignalingl)
libm_hidden_proto (__expl)
libm_hidden_proto (__expm1l)
# endif

# if __HAVE_DISTINCT_FLOAT128
libm_hidden_proto (__fpclassifyf128)
libm_hidden_proto (__issignalingf128)
libm_hidden_proto (__expf128)
libm_hidden_proto (__expm1f128)
# endif

# if __HAVE_DISTINCT_FLOAT128

/* __builtin_isinf_sign is broken in GCC < 7 for float128.  */
#  if ! __GNUC_PREREQ (7, 0)
#   include <ieee754_float128.h>
extern inline int
__isinff128 (_Float128 x)
{
  int64_t hx, lx;
  GET_FLOAT128_WORDS64 (hx, lx, x);
  lx |= (hx & 0x7fffffffffffffffLL) ^ 0x7fff000000000000LL;
  lx |= -lx;
  return ~(lx >> 63) & (hx >> 62);
}
#  endif

extern inline _Float128
fabsf128 (_Float128 x)
{
  return __builtin_fabsf128 (x);
}
# endif

# if !(defined __FINITE_MATH_ONLY__ && __FINITE_MATH_ONLY__ > 0)
#  ifndef NO_MATH_REDIRECT
/* Declare sqrt for use within GLIBC.  Compilers typically inline sqrt as a
   single instruction.  Use an asm to avoid use of PLTs if it doesn't.  */
float (sqrtf) (float) asm ("__ieee754_sqrtf");
double (sqrt) (double) asm ("__ieee754_sqrt");
#   ifndef __NO_LONG_DOUBLE_MATH
long double (sqrtl) (long double) asm ("__ieee754_sqrtl");
#   endif
#   if __HAVE_DISTINCT_FLOAT128 > 0
_Float128 (sqrtf128) (_Float128) asm ("__ieee754_sqrtf128");
#   endif
#  endif
# endif

#endif
#endif
