/* Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
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

#ifndef	_SYS_CDEFS_H

#define	_SYS_CDEFS_H	1
#include <features.h>

/* Some user header file might have defined this before.  */
#undef	__P

#ifdef __GNUC__

#define	__P(args)	args	/* GCC can always grok prototypes.  */
#define	__DOTS		, ...

/* In GCC versions before 2.5, the `volatile' and `const' keywords have
   special meanings when applied to functions.  In versions 2.5 and 2.6,
   the `__attribute__' syntax used below does not work properly.  */
#if	__GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5)
#define	__NORETURN	__volatile
#define	__CONSTVALUE	__const
 #elif	__GNUC__ > 2 || __GNUC_MINOR__ >= 7 /* Faith.  */
/* In GCC 2.5 and later, these keywords are meaningless when applied to
   functions, as ANSI requires.  Instead, we use GCC's special
   `__attribute__' syntax.  */
#define	__NORETURN	__attribute__ ((__volatile__))
#define	__CONSTVALUE	__attribute__ ((__const__))
#else
#define __NORETURN
#define __CONSTVALUE
#endif

#else	/* Not GCC.  */

#define	__inline		/* No inline functions.  */
#define	__NORETURN		/* No way to say functions never return.  */
#define	__CONSTVALUE		/* No way to say functions are functional.  */

#if (defined (__STDC__) && __STDC__) || defined (__cplusplus)

#define	__P(args)	args
#define	__const		const
#define	__signed	signed
#define	__volatile	volatile
#define	__DOTS		, ...

#else	/* Not ANSI C or C++.  */

#define	__P(args)	()	/* No prototypes.  */
#define	__const			/* No ANSI C keywords.  */
#define	__signed
#define	__volatile
#define	__DOTS

#endif	/* ANSI C or C++.  */

#endif	/* GCC.  */

/* For these things, GCC behaves the ANSI way normally,
   and the non-ANSI way under -traditional.  */

#if defined (__STDC__) && __STDC__

#define	__CONCAT(x,y)	x ## y
#define	__STRING(x)	#x

/* This is not a typedef so `const __ptr_t' does the right thing.  */
#define __ptr_t void *
typedef long double __long_double_t;

#else

#define	__CONCAT(x,y)	x/**/y
#define	__STRING(x)	"x"

#define __ptr_t char *
typedef double __long_double_t;

#endif

/* The BSD header files use the ANSI keywords unmodified.  (This means that
   old programs may lose if they use the new keywords as identifiers.)  We
   define them to their __ versions, which are taken care of above.  */

#ifdef	__USE_BSD
#define	const		__const
#define	signed		__signed
#define	volatile	__volatile
#endif

/* C++ needs to know that types and declarations are C, not C++.  */
#ifdef	__cplusplus
#define	__BEGIN_DECLS	extern "C" {
#define	__END_DECLS	}
#else
#define	__BEGIN_DECLS
#define	__END_DECLS
#endif

#endif	 /* sys/cdefs.h */
