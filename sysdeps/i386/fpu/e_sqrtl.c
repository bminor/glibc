/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <math_private.h>
#include <libm-alias-finite.h>

#undef __ieee754_sqrtl
long double
__ieee754_sqrtl (long double x)
{
  long double res;

  asm ("fsqrt" : "=t" (res) : "0" (x));

  return res;
}
libm_alias_finite (__ieee754_sqrtl, __sqrtl)
