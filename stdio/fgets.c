/* Copyright (C) 1991 Free Software Foundation, Inc.
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
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* Get a newline-terminated string of not more than
   N - 1 characters from `stream' and put it in S.  */
char *
DEFUN(fgets, (s, n, stream), char *s AND size_t n AND register FILE *stream)
{
  register char *p = s;

  if (!__validfp(stream))
    {
      errno = EINVAL;
      return NULL;
    }

  if (s == NULL || n == 0 || ferror(stream))
    return NULL;

  if (stream->__buffer == NULL && stream->__userbuf)
    {
      /* Unbuffered stream.  Not much optimization to do.  */
      register int c;
      while (--n > 0 && (c = getc(stream)) != EOF) 
	if ((*p++ = c) == '\n')
	  break;
      *p = '\0';
      return s;
    }

  if (!stream->__seen || stream->__buffer == NULL || stream->__pushed_back)
    {
      /* Do one with getc to allocate a buffer.  */
      int c = getc (stream);
      if (c == EOF)
	return NULL;
      *p++ = c;
      if (c == '\n')
	{
	  *p = '\0';
	  return s;
	}
    }

  while (n > 0)
    {
      size_t i;
      char *found;

      i = stream->__get_limit - stream->__bufp;	
      if (i == 0)
	{
	  /* Refill the buffer.  */
	  int c = __fillbf(stream);
	  if (c == EOF)
	    break; 
	  *p++ = c;
	  if (c == '\n')
	    {
	      *p = '\0';
	      return s;
	    }
	}

      if (i > n)
	i = n;

      found = (char *) __memccpy((PTR) p, stream->__bufp, '\n', i);

      if (found != NULL)
	{
	  stream->__bufp += found - p;
	  p = found;
	  break;
	}

      stream->__bufp += i;
      n -= i;
      p += i;
    }

  *p = '\0';
  return (p == s || ferror(stream)) ? NULL : s;
}
