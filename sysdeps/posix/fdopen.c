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


/* Defined in fopen.c.  */
extern int EXFUN(__getmode, (CONST char *mode, __io_mode *mptr));

/* Open a new stream on a given system file descriptor.  */
FILE *
DEFUN(fdopen, (fd, mode), int fd AND CONST char *mode)
{
  register FILE *stream;
  __io_mode m;

  if (fd < 0)
    {
      errno = EBADF;
      return NULL;
    }

  if (!__getmode(mode, &m))
    return NULL;

  stream = __newstream();
  if (stream == NULL)
    return NULL;

  stream->__fileno = fd;
  stream->__cookie = (PTR) &stream->__fileno;
  stream->__mode = m;

  return stream;
}
