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


/* Replace STREAM, opening it on FILENAME.  */
FILE *
DEFUN(freopen, (filename, mode, stream),
      CONST char *filename AND CONST char *mode AND register FILE *stream)
{
  FILE *head;

  if (!__validfp(stream))
    {
      errno = EINVAL;
      return NULL;
    }

  /* Return value explicitly ignored.  */
  (void) fclose(stream);

  /* Make sure STREAM will be the first one checked.  */
  head = __stdio_head;
  __stdio_head = stream;

  /* This will return either STREAM or NULL.  */
  stream = fopen(filename, mode);

  /* Restore the saved value.	*/
  __stdio_head = head;

  return stream;
}
