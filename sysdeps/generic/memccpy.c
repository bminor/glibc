/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility for
the consequences of using it or for whether it serves any particular
purpose or works at all, unless he says so in writing.  Refer to the GNU
C Library General Public License (in the file COPYING) for full details.

Everyone is granted permission to copy, modify and redistribute
the GNU C Library, but only under the conditions described in the
GNU C Library General Public License.  Among other things, this notice
must not be changed and a copy of the license must be included.  */

#include <ansidecl.h>
#include <stddef.h>	/* For size_t and NULL.	*/


/*
 * Copy no more than N bytes of SRC to DEST, stopping when C is found.
 * Return the position in DEST one byte past where C was copied,
 * or NULL if C was not found in the first N bytes of SRC.
 */
PTR
DEFUN(__memccpy, (dest, src, c, n),
      PTR dest AND CONST PTR src AND int c AND size_t n)
{
  register CONST char *s = src;
  register char *d = dest;
  register CONST int x = (unsigned char) c;
  register size_t i = n;

  while (i-- > 0)
    if ((*d++ = *s++) == x)
      return d;

  return NULL;
}
