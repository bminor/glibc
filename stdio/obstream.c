/* Copyright (C) 1992 Free Software Foundation, Inc.
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
#include <stdio.h>
#include <obstack.h>

FILE *obstack_stream __P ((struct obstack *));

static void
DEFUN(grow, (stream, c), FILE *stream AND int c)
{
  struct obstack *const obstack = (struct obstack *) stream->__cookie;
  const size_t bufp = stream->__bufp - stream->__buffer;

  /* Move the end of the object back to include only the portion
     of the buffer which the user has already written into.  */ 
  obstack_blank_fast (obstack, - (stream->__put_limit - stream->__bufp));

  if (c != EOF)
    obstack_1grow (obstack, (unsigned char) c);

  /* The stream buffer always maps exactly to the object on the top
     of the obstack.  The start of the buffer is the start of the object.
     The put limit points just past the end of the object.  On fflush, the
     obstack is sync'd so the end of the object points just past the last
     character written to the stream.  */

  stream->__buffer = obstack_base (obstack);
  stream->__bufsize += obstack_room (obstack);
  obstack_blank_fast (obstack, stream->__bufsize);
  stream->__bufp = stream->__buffer + bufp;
  stream->__get_limit = stream->__bufp;
  stream->__put_limit = stream->__buffer + stream->__bufsize;
}

FILE *
open_obstack_stream (obstack)
     struct obstack *obstack;
{
  register FILE *stream;

  stream = __newstream ();
  if (stream == NULL)
    return NULL;

  stream->__mode.__write = 1;
  stream->__mode.__read = 1;

  /* Input gets EOF.  */
  stream->__room_funcs.__input = NULL;

  /* When the buffer is full, grow the obstack.  */
  stream->__room_funcs.__output = grow;

  /* Do nothing for close.  */
  stream->__io_funcs.__close = NULL;

  /* Can't seek outside the buffer.  */
  stream->__io_funcs.__seek = NULL;
  stream->__target = stream->__offset = 0;

  stream->__seen = 1;

  /* Don't deallocate that buffer!  */
  stream->__userbuf = 1;

  /* We don't have to initialize the buffer.
     The first read attempt will call grow, which will do all the work.  */

  return stream;
}
