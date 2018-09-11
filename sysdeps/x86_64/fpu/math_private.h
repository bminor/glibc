#ifndef X86_64_MATH_PRIVATE_H
#define X86_64_MATH_PRIVATE_H 1

#include_next <math_private.h>

#ifdef __SSE4_1__
extern __always_inline double
__rint (double d)
{
  double res;
# if defined __AVX__ || defined SSE2AVX
  asm ("vroundsd $4, %1, %0, %0" : "=x" (res) : "xm" (d));
# else
  asm ("roundsd $4, %1, %0" : "=x" (res) : "xm" (d));
# endif
  return res;
}

extern __always_inline float
__rintf (float d)
{
  float res;
# if defined __AVX__ || defined SSE2AVX
  asm ("vroundss $4, %1, %0, %0" : "=x" (res) : "xm" (d));
# else
  asm ("roundss $4, %1, %0" : "=x" (res) : "xm" (d));
# endif
  return res;
}

extern __always_inline double
__floor (double d)
{
  double res;
# if defined __AVX__ || defined SSE2AVX
  asm ("vroundsd $1, %1, %0, %0" : "=x" (res) : "xm" (d));
# else
  asm ("roundsd $1, %1, %0" : "=x" (res) : "xm" (d));
# endif
  return res;
}

extern __always_inline float
__floorf (float d)
{
  float res;
# if defined __AVX__ || defined SSE2AVX
  asm ("vroundss $1, %1, %0, %0" : "=x" (res) : "xm" (d));
# else
  asm ("roundss $1, %1, %0" : "=x" (res) : "xm" (d));
#  endif
  return res;
}
#endif /* __SSE4_1__ */

#endif /* X86_64_MATH_PRIVATE_H */
