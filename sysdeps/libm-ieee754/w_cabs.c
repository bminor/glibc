/*
 * cabs() wrapper for hypot().
 *
 * Written by J.T. Conklin, <jtc@wimsey.com>
 * Placed into the Public Domain, 1994.
 */

#include <math.h>

struct __cabs_complex
{
  double x, y;
};

double
__cabs (struct __cabs_complex z)
{
	return __hypot(z.x, z.y);
}
weak_alias (__cabs, cabs)

#ifdef NO_LONG_DOUBLE
struct __cabs_complexl
{
  double x, y;
};

double
__cabsl (struct __cabs_complexl z)
{
	return __hypot(z.x, z.y);
}
weak_alias (__cabsl, cabsl)
#endif
