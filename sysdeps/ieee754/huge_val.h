/* `HUGE_VAL' constant for IEEE 754 machines (where it is infinity).
   Used by <stdlib.h> and <math.h> functions for overflow.

Copyright (C) 1992 Free Software Foundation, Inc.
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

#ifndef	   _HUGE_VAL_H
#define	   _HUGE_VAL_H	1

#include <sys/cdefs.h>
#include <endian.h>

/* IEEE positive infinity.  */

#if __BYTE_ORDER == __BIG_ENDIAN
#define	__huge_val_bytes	{ 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 }
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define	__huge_val_bytes	{ 0, 0, 0, 0, 0, 0, 0xf0, 0x7f }
#endif

#ifdef	__GNUC__
#define	HUGE_VAL \
  (__extension__ ((union { unsigned char __c[8];			      \
			   double __d; })				      \
		  { __huge_val_bytes }).__d)
#else	/* Not GCC.  */
static __const char __huge_val[8] = __huge_val_bytes;
#define	HUGE_VAL	(*(__const double *) __huge_val)
#endif	/* GCC.  */

#endif	   /* huge_val.h */
