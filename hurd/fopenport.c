/* Copyright (C) 1994 Free Software Foundation, Inc.
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
#include <hurd.h>
#include <stdio.h>
#include <fcntl.h>


/* Defined in fopen.c.  */
extern int EXFUN(__getmode, (CONST char *mode, __io_mode *mptr));


/* Open a stream on PORT.  MODE is as for fopen.  */

FILE *
__fopenport (mach_port_t port, const char *mode)
{
  register FILE *stream;
  __io_mode m;
  int pflags;
  error_t err;

  if (!__getmode (mode, &m))
    return NULL;

  /* Verify the PORT is valid allows the access MODE specifies.  */

  if (err = __io_get_openmodes (port, &pflags))
    return __hurd_fail (err), NULL;

  /* Check the access mode.  */
  if ((pflags & O_READ && !m.__read) || (pflags & O_WRITE && !m.__write))
    {
      errno = EBADF;		/* XXX ? */
      return NULL;
    }

  stream = __newstream ();
  if (stream == NULL)
    return NULL;

  /* The default io functions in sysd-stdio.c use Hurd io ports as cookies.  */

  stream->__cookie = (PTR) port;
  stream->__mode = m;

  return stream;
}
