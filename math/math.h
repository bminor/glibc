/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	ANSI Standard: 4.5 MATHEMATICS	<math.h>
 */

#ifndef	_MATH_H

#define	_MATH_H	1
#include <features.h>


#define	__need_Emath
#include <errno.h>

/* Get machine-dependent HUGE_VAL value (returned on overflow).  */
#include <huge_val.h>

/* Get machine-dependent NAN value (returned for some domain errors).  */
#ifdef	 __USE_GNU
#include <nan.h>
#endif


#ifndef	__CONSTVALUE
#ifdef	__GNUC__
/* The `const' keyword tells GCC that a function's return value is
   based solely on its arguments, and there are no side-effects.  */
#define	__CONSTVALUE	__const
#else
#define	__CONSTVALUE
#endif	/* GCC.  */
#endif	/* __CONSTVALUE not defined.  */


/* Trigonometric functions.  */

/* Arc cosine of X.  */
extern __CONSTVALUE double EXFUN(acos, (double __x));
/* Arc sine of X.  */
extern __CONSTVALUE double EXFUN(asin, (double __x));
/* Arc tangent of X.  */
extern __CONSTVALUE double EXFUN(atan, (double __x));
/* Arc tangent of Y/X.  */
extern __CONSTVALUE double EXFUN(atan2, (double __y, double __x));

/* Cosine of X.  */
extern __CONSTVALUE double EXFUN(cos, (double __x));
/* Sine of X.  */
extern __CONSTVALUE double EXFUN(sin, (double __x));
/* Tangent of X.  */
extern __CONSTVALUE double EXFUN(tan, (double __x));


/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
extern __CONSTVALUE double EXFUN(cosh, (double __x));
/* Hyperbolic sine of X.  */
extern __CONSTVALUE double EXFUN(sinh, (double __x));
/* Hyperbolic tangent of X.  */
extern __CONSTVALUE double EXFUN(tanh, (double __x));

#ifdef	__USE_MISC
/* Hyperbolic arc cosine of X.  */
extern __CONSTVALUE double EXFUN(acosh, (double __x));
/* Hyperbolic arc sine of X.  */
extern __CONSTVALUE double EXFUN(asinh, (double __x));
/* Hyperbolic arc tangent of X.  */
extern __CONSTVALUE double EXFUN(atanh, (double __x));
#endif

/* Exponential and logarithmic functions.  */

/* Exponentional function of X.  */
extern __CONSTVALUE double EXFUN(exp, (double __x));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
extern double EXFUN(frexp, (double __value, int *__exp));

/* X times (two to the EXP power).  */
extern __CONSTVALUE double EXFUN(ldexp, (double __x, int __exp));

/* Natural logarithm of X.  */
extern __CONSTVALUE double EXFUN(log, (double __x));

/* Base-ten logarithm of X.  */
extern __CONSTVALUE double EXFUN(log10, (double __x));

#ifdef	__USE_MISC
/* Return exp(X) - 1.  */
extern __CONSTVALUE double EXFUN(expm1, (double __x));

/* Return log(1 + X).  */
extern __CONSTVALUE double EXFUN(log1p, (double __x));
#endif

/* Break VALUE into integral and fractional parts.  */
extern double EXFUN(modf, (double __value, double *__iptr));


/* Power functions.  */

/* Return X to the Y power.  */
extern __CONSTVALUE double EXFUN(pow, (double __x, double __y));

/* Return the square root of X.  */
extern __CONSTVALUE double EXFUN(sqrt, (double __x));

#ifdef	__USE_MISC
/* Return the cube root of X.  */
extern __CONSTVALUE double EXFUN(cbrt, (double __x));
#endif


/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
extern __CONSTVALUE double EXFUN(ceil, (double __x));

/* Absolute value of X.  */
extern __CONSTVALUE double EXFUN(fabs, (double __x));

/* Largest integer not greater than X.  */
extern __CONSTVALUE double EXFUN(floor, (double __x));

/* Floating-point modulo remainder of X/Y.  */
extern __CONSTVALUE double EXFUN(fmod, (double __x, double __y));


/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
extern __CONSTVALUE int EXFUN(__isinf, (double __value));

/* Return nonzero if VALUE is not a number.  */
extern __CONSTVALUE int EXFUN(__isnan, (double __value));

/* Return nonzero if VALUE is finite and not NaN.  */
extern __CONSTVALUE int EXFUN(__finite, (double __value));
#ifdef	__OPTIMIZE__
#define	__finite(value)	(!__isinf(value))
#endif

/* Deal with an infinite or NaN result.
   If ERROR is ERANGE, result is +Inf;
   if ERROR is - ERANGE, result is -Inf;
   otherwise result is NaN.
   This will set `errno' to either ERANGE or EDOM,
   and may return an infinity or NaN, or may do something else.  */
extern double EXFUN(__infnan, (int __error));

/* Return X with its signed changed to Y's.  */
extern __CONSTVALUE double EXFUN(__copysign, (double __x, double __y));

/* Return X times (2 to the Nth power).  */
extern __CONSTVALUE double EXFUN(__scalb, (double __x, int __n));

#ifdef	__OPTIMIZE__
#define	__scalb(x, n)	ldexp ((x), (n))
#endif

/* Return the remainder of X/Y.  */
extern __CONSTVALUE double EXFUN(__drem, (double __x, double __y));

/* Return the base 2 signed integral exponent of X.  */
extern __CONSTVALUE double EXFUN(__logb, (double __x));

#ifdef	__USE_MISC

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern __CONSTVALUE double EXFUN(rint, (double __x));

/* Return `sqrt(X*X + Y*Y)'.  */
extern __CONSTVALUE double EXFUN(hypot, (double __x, double __y));

struct __complex
  {
    double __x, __y;
  };
/* Return `sqrt(X*X + Y*Y)'.  */
extern __CONSTVALUE double EXFUN(cabs, (struct __complex));

extern __CONSTVALUE int EXFUN(isinf, (double __value));
extern __CONSTVALUE int EXFUN(isnan, (double __value));
extern __CONSTVALUE int EXFUN(finite, (double __value));
extern __CONSTVALUE double EXFUN(infnan, (int __error));
extern __CONSTVALUE double EXFUN(copysign, (double __x, double __y));
extern __CONSTVALUE double EXFUN(scalb, (double __x, int __n));
extern __CONSTVALUE double EXFUN(drem, (double __x, double __y));
extern __CONSTVALUE double EXFUN(logb, (double __x));

#ifdef	__OPTIMIZE__
#define	isinf(value)	__isinf(value)
#define	isnan(value)	__isnan(value)
#define	infnan(error)	__infnan(error)
#define	finite(value)	__finite(value)
#define	copysign(x, y)	__copysign((x), (y))
#define	scalb(x, n)	__scalb((x), (n))
#define	drem(x, y)	__drem((x), (y))
#define	logb(x)		__logb(x)
#endif	/* Optimizing.  */

#endif	/* Use misc.  */


#if 0
/* The "Future Library Directions" section of the
   ANSI Standard reserves these as `float' and
   `long double' versions of the above functions.  */

extern __CONSTVALUE float EXFUN(acosf, (float __x));
extern __CONSTVALUE float EXFUN(asinf, (float __x));
extern __CONSTVALUE float EXFUN(atanf, (float __x));
extern __CONSTVALUE float EXFUN(atan2f, (float __y, float __x));
extern __CONSTVALUE float EXFUN(cosf, (float __x));
extern __CONSTVALUE float EXFUN(sinf, (float __x));
extern __CONSTVALUE float EXFUN(tanf, (float __x));
extern __CONSTVALUE float EXFUN(coshf, (float __x));
extern __CONSTVALUE float EXFUN(sinhf, (float __x));
extern __CONSTVALUE float EXFUN(tanhf, (float __x));
extern __CONSTVALUE float EXFUN(expf, (float __x));
extern float EXFUN(frexpf, (float __value, int *__exp));
extern __CONSTVALUE float EXFUN(ldexpf, (float __x, int __exp));
extern __CONSTVALUE float EXFUN(logf, (float __x));
extern __CONSTVALUE float EXFUN(log10f, (float __x));
extern float EXFUN(modff, (float __value, float *__iptr));
extern __CONSTVALUE float EXFUN(powf, (float __x, float __y));
extern __CONSTVALUE float EXFUN(sqrtf, (float __x));
extern __CONSTVALUE float EXFUN(ceilf, (float __x));
extern __CONSTVALUE float EXFUN(fabsf, (float __x));
extern __CONSTVALUE float EXFUN(floorf, (float __x));
extern __CONSTVALUE float EXFUN(fmodf, (float __x, float __y));

extern __CONSTVALUE LONG_DOUBLE EXFUN(acosl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(asinl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(atanl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(atan2l,
				      (LONG_DOUBLE __y, LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(cosl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(sinl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(tanl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(coshl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(sinhl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(tanhl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(expl, (LONG_DOUBLE __x));
extern LONG_DOUBLE EXFUN(frexpl, (LONG_DOUBLE __value, int *__exp));
extern __CONSTVALUE LONG_DOUBLE EXFUN(ldexpl, (LONG_DOUBLE __x, int __exp));
extern __CONSTVALUE LONG_DOUBLE EXFUN(logl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(log10l, (LONG_DOUBLE __x));
extern LONG_DOUBLE EXFUN(modfl, (LONG_DOUBLE __value, LONG_DOUBLE *__ip));
extern __CONSTVALUE LONG_DOUBLE EXFUN(powl,
				      (LONG_DOUBLE __x, LONG_DOUBLE __y));
extern __CONSTVALUE LONG_DOUBLE EXFUN(sqrtl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(ceill, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(fabsl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(floorl, (LONG_DOUBLE __x));
extern __CONSTVALUE LONG_DOUBLE EXFUN(fmodl,
				      (LONG_DOUBLE __x, LONG_DOUBLE __y));
#endif	/* 0 */

/* Get machine-dependent inline versions (if there are any).  */
#include <__math.h>

#endif	/* math.h  */
