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
#include <stdio.h>
#include <stdlib.h>


struct memstream_info
  {
    char **buffer;
    size_t *bufsize;
  };

/* Enlarge STREAM's buffer.  */
static void
DEFUN(enlarge_buffer, (stream, c),
      register FILE *stream AND int c)
{
  struct memstream_info *info = (struct memstream_info *) stream->__cookie;
  char *newbuf;

  *info->bufsize = stream->__bufp - stream->__buffer;
  if (stream->__put_limit - stream->__bufp < 1)
    {
      stream->__bufsize += 100;
      newbuf = (char *) realloc ((PTR) stream->__buffer, stream->__bufsize);
      *info->buffer = newbuf;
      if (newbuf == NULL)
	{
	  free ((PTR) stream->__buffer);
	  stream->__buffer = stream->__bufp
	    = stream->__put_limit = stream->__get_limit = NULL;
	  stream->__error = 1;
	  return;
	}

      stream->__buffer = newbuf;
      stream->__bufp = stream->__buffer + *info->bufsize;
      stream->__get_limit = stream->__put_limit;
      stream->__put_limit = stream->__buffer + stream->__bufsize;
    }

  if (c != EOF)
    *stream->__bufp++ = (unsigned char) c;
  else
    *stream->__bufp = '\0';
}

static int
DEFUN(free_info, (cookie), PTR cookie)
{
#if 0
  struct memstream_info *info = (struct memstream_info *) cookie;
  char *buf;

  buf = (char *) realloc ((PTR) *info->buffer, *info->bufsize);
  if (buf != NULL)
    *info->buffer = buf;
#endif

  free (cookie);

  return 0;
}

/* Open a stream that writes into a malloc'd buffer that is expanded as
   necessary.  *BUFLOC and *SIZELOC are updated with the buffer's location
   and the number of characters written on fflush or fclose.  */
FILE *
DEFUN(open_memstream, (bufloc, sizeloc),
      char **bufloc AND size_t *sizeloc)
{
  FILE *stream;
  struct memstream_info *info;

  if (bufloc == NULL || sizeloc == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  stream = fmemopen ((char *) NULL, BUFSIZ, "w+");
  if (stream == NULL)
    return NULL;

  info = (struct memstream_info *) malloc (sizeof (struct memstream_info));
  if (info == NULL)
    {
      int save = errno;
      (void) fclose (stream);
      errno = save;
      return NULL;
    }

  stream->__room_funcs.__output = enlarge_buffer;
  stream->__io_funcs.__close = free_info;
  stream->__cookie = (PTR) info;
  stream->__userbuf = 1;

  info->buffer = bufloc;
  info->bufsize = sizeloc;

  *bufloc = stream->__buffer;

  return stream;
}
