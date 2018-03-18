/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Changes for long double by Ulrich Drepper <drepper@cygnus.com>
 * Public domain.
 */

#include <math.h>
#include <math_private.h>
#include <libm-alias-ldouble.h>

long double
__rintl (long double x)
{
  long double res;

  asm ("frndint" : "=t" (res) : "0" (x));
  return res;
}
libm_hidden_def (__rintl)
libm_alias_ldouble (__rint, rint)
