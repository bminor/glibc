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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <stddef.h>
#include <string.h>


/* Return the first occurrence of NEEDLE in HAYSTACK.  */
PTR
DEFUN(memmem, (haystack, haystack_len,
	       needle, needle_len),
      CONST PTRCONST haystack AND CONST size_t haystack_len AND
      CONST PTRCONST needle AND CONST size_t needle_len)
{
  register CONST char *begin;

  if (needle_len == 0)
    return (PTR) &((CONST char *) haystack)[needle_len - 1];

  for (begin = &((CONST char *) haystack)[needle_len - 1];
       begin < (CONST char *) haystack + haystack_len;
       ++begin)
    if (!memcmp ((PTR) begin, needle, needle_len))
      return (PTR) begin;

  return NULL;
}
