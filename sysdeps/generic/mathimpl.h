/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)mathimpl.h	5.3 (Berkeley) 10/9/90
 */

#include <math.h>

#include <endian.h>
#ifdef	__LITTLE_ENDIAN
#undef national
#define national
#endif


#undef	isinf
#define	isinf	__isinf
#undef	isnan
#define	isnan	__isnan
#undef	infnan
#define	infnan	__infnan
#undef	copysign
#define	copysign	__copysign
#undef	scalb
#define	scalb	__scalb
#undef	drem
#define	drem	__drem
#undef	logb
#define	logb	__logb
#undef	__finite
#undef	finite
#define	finite	__finite
#undef	expm1
#define	expm1	__expm1

#define	exp__E	__exp__E
#define	log__L	__log__L

#include <sys/cdefs.h>

#if defined(vax)||defined(tahoe)

/* Deal with different ways to concatenate in cpp */
#  ifdef __STDC__
#    define	cat3(a,b,c) a ## b ## c
#  else
#    define	cat3(a,b,c) a/**/b/**/c
#  endif

/* Deal with vax/tahoe byte order issues */
#  ifdef vax
#    define	cat3t(a,b,c) cat3(a,b,c)
#  else
#    define	cat3t(a,b,c) cat3(a,c,b)
#  endif

#  define vccast(name) (*(const double *)(cat3(name,,x)))

   /*
    * Define a constant to high precision on a Vax or Tahoe.
    *
    * Args are the name to define, the decimal floating point value,
    * four 16-bit chunks of the float value in hex
    * (because the vax and tahoe differ in float format!), the power
    * of 2 of the hex-float exponent, and the hex-float mantissa.
    * Most of these arguments are not used at compile time; they are
    * used in a post-check to make sure the constants were compiled
    * correctly.
    *
    * People who want to use the constant will have to do their own
    *     #define foo vccast(foo)
    * since CPP cannot do this for them from inside another macro (sigh).
    * We define "vccast" if this needs doing.
    */
#  define vc(name, value, x1,x2,x3,x4, bexp, xval) \
	const static long cat3(name,,x)[] = {cat3t(0x,x1,x2), cat3t(0x,x3,x4)};

#  define ic(name, value, bexp, xval) ;

#else	/* vax or tahoe */

   /* Hooray, we have an IEEE machine */
#  undef vccast
#  define vc(name, value, x1,x2,x3,x4, bexp, xval) ;

#  define ic(name, value, bexp, xval) \
	const static double name = value;

#endif	/* defined(vax)||defined(tahoe) */


/*
 * Functions internal to the math package, yet not static.
 */
extern double	exp__E();
extern double	log__L();

