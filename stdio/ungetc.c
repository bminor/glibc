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


/* Push the character C back onto the input stream of STREAM.  */
int
DEFUN(ungetc, (c, stream), register int c AND register FILE *stream)
{
  if (!__validfp(stream) || !stream->__mode.__read)
    {
      errno = EINVAL;
      return EOF;
    }

  if (c == EOF)
    return EOF;

  if (stream->__pushback_bufp != NULL)
    /* There is already a char pushed back.  */
    return EOF;

  stream->__pushback = (unsigned char) c;
  /* Tell __fillbf we've pushed back a char.  */
  stream->__pushback_bufp = stream->__bufp;
  /* Make the next getc call __fillbf.  It will return C.  */
  stream->__bufp = stream->__get_limit;

  /* We just gave it another character to read, so it's not at EOF.  */
  stream->__eof = 0;

  return stream->__pushback;
}
