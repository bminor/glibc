/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	ANSI Standard: 4.5 MATHEMATICS	<math.h>
 */

#ifndef	_MATH_H

#define	_MATH_H	1
#include <features.h>

__BEGIN_DECLS

#define	__need_Emath
#include <errno.h>

/* Get machine-dependent HUGE_VAL value (returned on overflow).  */
#include <huge_val.h>

/* Get machine-dependent NAN value (returned for some domain errors).  */
#ifdef	 __USE_GNU
#include <nan.h>
#endif


/* Trigonometric functions.  */

/* Arc cosine of X.  */
extern __CONSTVALUE double acos __P ((double __x));
/* Arc sine of X.  */
extern __CONSTVALUE double asin __P ((double __x));
/* Arc tangent of X.  */
extern __CONSTVALUE double atan __P ((double __x));
/* Arc tangent of Y/X.  */
extern __CONSTVALUE double atan2 __P ((double __y, double __x));

/* Cosine of X.  */
extern __CONSTVALUE double cos __P ((double __x));
/* Sine of X.  */
extern __CONSTVALUE double sin __P ((double __x));
/* Tangent of X.  */
extern __CONSTVALUE double tan __P ((double __x));


/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
extern __CONSTVALUE double cosh __P ((double __x));
/* Hyperbolic sine of X.  */
extern __CONSTVALUE double sinh __P ((double __x));
/* Hyperbolic tangent of X.  */
extern __CONSTVALUE double tanh __P ((double __x));

#ifdef	__USE_MISC
/* Hyperbolic arc cosine of X.  */
extern __CONSTVALUE double acosh __P ((double __x));
/* Hyperbolic arc sine of X.  */
extern __CONSTVALUE double asinh __P ((double __x));
/* Hyperbolic arc tangent of X.  */
extern __CONSTVALUE double atanh __P ((double __x));
#endif

/* Exponential and logarithmic functions.  */

/* Exponentional function of X.  */
extern __CONSTVALUE double exp __P ((double __x));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
extern double frexp __P ((double __value, int *__exp));

/* X times (two to the EXP power).  */
extern __CONSTVALUE double ldexp __P ((double __x, int __exp));

/* Natural logarithm of X.  */
extern __CONSTVALUE double log __P ((double __x));

/* Base-ten logarithm of X.  */
extern __CONSTVALUE double log10 __P ((double __x));

#ifdef	__USE_MISC
/* Return exp(X) - 1.  */
extern __CONSTVALUE double __expm1 __P ((double __x));
extern __CONSTVALUE double expm1 __P ((double __x));

/* Return log(1 + X).  */
extern __CONSTVALUE double log1p __P ((double __x));
#endif

/* Break VALUE into integral and fractional parts.  */
extern double modf __P ((double __value, double *__iptr));


/* Power functions.  */

/* Return X to the Y power.  */
extern __CONSTVALUE double pow __P ((double __x, double __y));

/* Return the square root of X.  */
extern __CONSTVALUE double sqrt __P ((double __x));

#ifdef	__USE_MISC
/* Return the cube root of X.  */
extern __CONSTVALUE double cbrt __P ((double __x));
#endif


/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
extern __CONSTVALUE double ceil __P ((double __x));

/* Absolute value of X.  */
extern __CONSTVALUE double fabs __P ((double __x));

/* Largest integer not greater than X.  */
extern __CONSTVALUE double floor __P ((double __x));

/* Floating-point modulo remainder of X/Y.  */
extern __CONSTVALUE double fmod __P ((double __x, double __y));


/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
extern __CONSTVALUE int __isinf __P ((double __value));

/* Return nonzero if VALUE is not a number.  */
extern __CONSTVALUE int __isnan __P ((double __value));

/* Return nonzero if VALUE is finite and not NaN.  */
extern __CONSTVALUE int __finite __P ((double __value));
#ifdef	__OPTIMIZE__
#define	__finite(value)	(!__isinf(value))
#endif

/* Deal with an infinite or NaN result.
   If ERROR is ERANGE, result is +Inf;
   if ERROR is - ERANGE, result is -Inf;
   otherwise result is NaN.
   This will set `errno' to either ERANGE or EDOM,
   and may return an infinity or NaN, or may do something else.  */
extern double __infnan __P ((int __error));

/* Return X with its signed changed to Y's.  */
extern __CONSTVALUE double __copysign __P ((double __x, double __y));

/* Return X times (2 to the Nth power).  */
extern __CONSTVALUE double __scalb __P ((double __x, int __n));

#ifdef	__OPTIMIZE__
#define	__scalb(x, n)	ldexp ((x), (n))
#endif

/* Return the remainder of X/Y.  */
extern __CONSTVALUE double __drem __P ((double __x, double __y));

/* Return the base 2 signed integral exponent of X.  */
extern __CONSTVALUE double __logb __P ((double __x));

#ifdef	__USE_MISC

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern __CONSTVALUE double __rint __P ((double __x));
extern __CONSTVALUE double rint __P ((double __x));

/* Return `sqrt(X*X + Y*Y)'.  */
extern __CONSTVALUE double hypot __P ((double __x, double __y));

struct __cabs_complex
{
  double __x, __y;
};

/* Return `sqrt(X*X + Y*Y)'.  */
extern __CONSTVALUE double cabs __P ((struct __cabs_complex));

extern __CONSTVALUE int isinf __P ((double __value));
extern __CONSTVALUE int isnan __P ((double __value));
extern __CONSTVALUE int finite __P ((double __value));
extern __CONSTVALUE double infnan __P ((int __error));
extern __CONSTVALUE double copysign __P ((double __x, double __y));
extern __CONSTVALUE double scalb __P ((double __x, int __n));
extern __CONSTVALUE double drem __P ((double __x, double __y));
extern __CONSTVALUE double logb __P ((double __x));

#ifdef	__OPTIMIZE__
#define	isinf(value)	__isinf(value)
#define	isnan(value)	__isnan(value)
#define	infnan(error)	__infnan(error)
#define	finite(value)	__finite(value)
#define	copysign(x, y)	__copysign((x), (y))
#define	scalb(x, n)	__scalb((x), (n))
#define	drem(x, y)	__drem((x), (y))
#define	logb(x)		__logb(x)
#endif /* Optimizing.  */

#endif /* Use misc.  */


#if 0
/* The "Future Library Directions" section of the
   ANSI Standard reserves these as `float' and
   `long double' versions of the above functions.  */

extern __CONSTVALUE float acosf __P ((float __x));
extern __CONSTVALUE float asinf __P ((float __x));
extern __CONSTVALUE float atanf __P ((float __x));
extern __CONSTVALUE float atan2f __P ((float __y, float __x));
extern __CONSTVALUE float cosf __P ((float __x));
extern __CONSTVALUE float sinf __P ((float __x));
extern __CONSTVALUE float tanf __P ((float __x));
extern __CONSTVALUE float coshf __P ((float __x));
extern __CONSTVALUE float sinhf __P ((float __x));
extern __CONSTVALUE float tanhf __P ((float __x));
extern __CONSTVALUE float expf __P ((float __x));
extern float frexpf __P ((float __value, int *__exp));
extern __CONSTVALUE float ldexpf __P ((float __x, int __exp));
extern __CONSTVALUE float logf __P ((float __x));
extern __CONSTVALUE float log10f __P ((float __x));
extern float modff __P ((float __value, float *__iptr));
extern __CONSTVALUE float powf __P ((float __x, float __y));
extern __CONSTVALUE float sqrtf __P ((float __x));
extern __CONSTVALUE float ceilf __P ((float __x));
extern __CONSTVALUE float fabsf __P ((float __x));
extern __CONSTVALUE float floorf __P ((float __x));
extern __CONSTVALUE float fmodf __P ((float __x, float __y));

extern __CONSTVALUE __long_double_t acosl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t asinl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t atanl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t atan2l __P ((__long_double_t __y, __long_double_t __x));
extern __CONSTVALUE __long_double_t cosl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t sinl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t tanl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t coshl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t sinhl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t tanhl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t expl __P ((__long_double_t __x));
extern __long_double_t frexpl __P ((__long_double_t __value, int *__exp));
extern __CONSTVALUE __long_double_t ldexpl __P ((__long_double_t __x, int __exp));
extern __CONSTVALUE __long_double_t logl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t log10l __P ((__long_double_t __x));
extern __long_double_t modfl __P ((__long_double_t __value, __long_double_t * __ip));
extern __CONSTVALUE __long_double_t powl __P ((__long_double_t __x, __long_double_t __y));
extern __CONSTVALUE __long_double_t sqrtl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t ceill __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t fabsl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t floorl __P ((__long_double_t __x));
extern __CONSTVALUE __long_double_t fmodl __P ((__long_double_t __x, __long_double_t __y));
#endif /* 0 */

/* Get machine-dependent inline versions (if there are any).  */
#include <__math.h>

__END_DECLS


#ifdef __USE_BSD
/* Some useful constants.  */
#define	M_E		2.7182818284590452354	/* e */
#define	M_LOG2E		1.4426950408889634074	/* log 2e */
#define	M_LOG10E	0.43429448190325182765	/* log 10e */
#define	M_LN2		0.69314718055994530942	/* log e2 */
#define	M_LN10		2.30258509299404568402	/* log e10 */
#define	M_PI		3.14159265358979323846	/* pi */
#define	M_PI_2		1.57079632679489661923	/* pi/2 */
#define	M_PI_4		0.78539816339744830962	/* pi/4 */
#define	M_1_PI		0.31830988618379067154	/* 1/pi */
#define	M_2_PI		0.63661977236758134308	/* 2/pi */
#define	M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
#define	M_SQRT2		1.41421356237309504880	/* sqrt(2) */
#define	M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */
#endif


#endif /* math.h  */
