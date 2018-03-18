#ifndef _COMPLEX_H
# include <math/complex.h>

# ifndef _ISOMAC
#  include <libm-symbols.h>

libm_hidden_mathcall (casin)
libm_hidden_mathcall (casinh)
libm_hidden_mathcall (ccosh)
libm_hidden_mathcall (cexp)
libm_hidden_mathcall (clog)
libm_hidden_mathcall (csqrt)

/* Return the complex inverse hyperbolic sine of finite nonzero Z,
   with the imaginary part of the result subtracted from pi/2 if ADJ
   is nonzero.  */
extern complex float __kernel_casinhf (complex float z, int adj)
  attribute_hidden;
extern complex double __kernel_casinh (complex double z, int adj)
  attribute_hidden;
extern complex long double __kernel_casinhl (complex long double z, int adj)
  attribute_hidden;
#  if __HAVE_DISTINCT_FLOAT128
extern __CFLOAT128 __kernel_casinhf128 (__CFLOAT128 z, int adj)
  attribute_hidden;
#  endif


# endif
#endif
