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
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	ANSI Standard: 4.2 DIAGNOSTICS	<assert.h>
 */

#ifdef	_ASSERT_H

#undef	_ASSERT_H
#undef	assert

#endif /* assert.h	*/

#define	_ASSERT_H	1
#include <features.h>

__BEGIN_DECLS

/* void assert(int expression);
   If NDEBUG is defined, do nothing.
   If not, and EXPRESSION is zero, print an error message and abort.  */

#ifdef	NDEBUG

#define	assert(expr)	((void) 0)

#else /* Not NDEBUG.  */

/* This prints an "Assertion failed" message and aborts.  */
extern int __assert_fail __P ((__const char *__assertion,
			       __const char *__file, unsigned int __line));

#include <sys/cdefs.h>

#define	assert(expr)							      \
  ((void) ((expr) ||							      \
	   __assert_fail (__STRING(expr), __FILE__, __LINE__)))

#endif /* NDEBUG.  */

__END_DECLS
