/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* IGNORE(@ */
#include <ansidecl.h>
#include <stddef.h>
/* @) */

#define	_MALLOC_INTERNAL
#include <malloc.h>

#ifndef	__GNU_LIBRARY__
#define	__sbrk	sbrk
#endif

extern PTR EXFUN(__sbrk, (int increment));

/* Allocate INCREMENT more bytes of data space,
   and return the start of data space, or NULL on errors.
   If INCREMENT is negative, shrink data space.  */
PTR
DEFUN(__default_morecore, (increment), long int increment)
{
  PTR result = __sbrk((int) increment);
  if (result == (PTR) -1)
    return NULL;
  return result;
}
