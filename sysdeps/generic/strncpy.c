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
#include <string.h>
#include <memcopy.h>

char *
DEFUN(strncpy, (s1, s2, n), char *s1 AND CONST char *s2 AND size_t n)
{
  reg_char c;
  char *s = s1;

  --s1;

  if (n >= 4)
    {
      size_t n4 = n >> 2;

      for (;;)
	{
	  c = *s2++;
	  *++s1 = c;
	  if (c == '\0')
	    break;
	  c = *s2++;
	  *++s1 = c;
	  if (c == '\0')
	    break;
	  c = *s2++;
	  *++s1 = c;
	  if (c == '\0')
	    break;
	  c = *s2++;
	  *++s1 = c;
	  if (c == '\0')
	    break;
	  if (--n4 == 0)
	    goto last_chars;
	}
      n = n - (s1 - s) - 1;
      if (n == 0)
	return s;
      goto zero_fill;
    }

 last_chars:
  n &= 3;
  if (n == 0)
    return s;

  do
    {
      c = *s2++;
      *++s1 = c;
      if (--n == 0)
	return s;
    }
  while (c != '\0');

 zero_fill:
  do
    *++s1 = '\0';
  while (--n > 0);

  return s;
}
