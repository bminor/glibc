/* Copyright (C) 1990 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef	_VARARGS_H

#define	_VARARGS_H	1
#include <features.h>

/* Implement varargs on top of our stdarg implementation.  */

#include <stdarg.h>

#define	va_alist	__va_fakearg
#define	va_dcl		int __va_fakearg;

#undef	va_start
#define	va_start(ap)	(__va_start((ap), __va_fakearg), \
			 (ap) -= sizeof(__va_fakearg))

#endif	/* varargs.h  */
