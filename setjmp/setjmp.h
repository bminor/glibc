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
 *	ANSI Standard: 4.6 NON-LOCAL JUMPS <setjmp.h>
 */

#ifndef	_SETJMP_H

#define	_SETJMP_H	1
#include <features.h>

/* Get the machine-dependent definition of `__jmp_buf'.  */
#include <jmp_buf.h>


#ifdef	__FAVOR_BSD

/* BSD defines `setjmp' and `longjmp' to save and restore the set of
   blocked signals.  For this, `jmp_buf' must be what POSIX calls
   `sigjmp_buf', which includes that information.  */
typedef	sigjmp_buf jmp_buf;

#else	/* Don't favor BSD.  */

/* A `jmp_buf' really is a `jmp_buf'.  Oh boy.  */
typedef	__jmp_buf jmp_buf;

#endif	/* Favor BSD.  */


#ifndef	__NORETURN
#ifdef	__GNUC__
/* The `volatile' keyword tells GCC that a function never returns.  */
#define	__NORETURN	__volatile
#else	/* Not GCC.  */
#define	__NORETURN
#endif	/* GCC.  */
#endif	/* __NORETURN not defined.  */

/* Jump to the environment saved in ENV, making the
   setjmp call there return VAL, or 1 if VAL is 0.  */
extern __NORETURN void EXFUN(__longjmp, (CONST __jmp_buf __env, int __val));
extern __NORETURN void EXFUN(longjmp, (CONST jmp_buf __env, int __val));

#ifdef	__OPTIMIZE__
#define	longjmp(env, val)	__longjmp ((env), (val))
#endif	/* Optimizing.  */

/* Set ENV to the current position and return 0.  */
extern int EXFUN(__setjmp, (__jmp_buf __env));
/* The ANSI C standard says `setjmp' is a macro.  */
#define	setjmp(env)	__setjmp (env)


#ifdef	__USE_POSIX
#define	__need_sigset_t
#include <signal.h>

/* Calling environment, plus possibly a saved signal mask.  */
typedef struct
  {
    __jmp_buf __jmpbuf;		/* Calling environment.  */
    int __savemask;		/* Saved the signal mask?  */
    sigset_t __sigmask;		/* Saved signal mask.  */
  } sigjmp_buf[1];

/* Store the calling environment in ENV, also saving the
   signal mask if SAVEMASK is nonzero.  Return 0.  */
extern int EXFUN(sigsetjmp, (sigjmp_buf __env, int __savemask));

/* Jump to the environment saved in ENV, making the
   sigsetjmp call there return VAL, or 1 if VAL is 0.
   Restore the signal mask if that sigsetjmp call saved it.  */
extern __NORETURN void EXFUN(siglongjmp, (CONST sigjmp_buf __env, int __val));
#endif	/* Use POSIX.  */


#ifdef	__USE_BSD
extern __NORETURN void EXFUN(_longjmp, (CONST jmp_buf __env, int __val));
extern int EXFUN(_setjmp, (jmp_buf __env));

#ifdef	__OPTIMIZE__
#define	_longjmp(env, val)	siglongjmp ((env), (val))
#define	_setjmp(env)		sigsetjmp ((env), 0)
#endif	/* Optimizing.  */
#endif	/* Use BSD.  */


#ifdef	__FAVOR_BSD
#undef	setjmp
#undef	longjmp
#define	setjmp(env)		sigsetjmp ((env), 1)
#define	longjmp(env, val)	siglongjmp ((env), (val))
#endif	/* Favor BSD.  */


#endif	/* setjmp.h  */
