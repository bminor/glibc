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

/* This file provides glue between Unix stdio and GNU stdio.
   It supports use of Unix stdio `getc' and `putc' (and,
   by extension, `getchar' and `putchar') macros on GNU stdio streams
   (they are slow, but they work).  It also supports all stdio
   operations (including Unix `getc' and `putc') on Unix's stdin, stdout,
   and stderr (the elements of `_iob').

   The reasoning behind this is to allow programs (and especially
   libraries) compiled with Unix header files to work with the
   GNU C library.  */

#include <ansidecl.h>
#include <stdio.h>
#include <errno.h>

typedef union
  {
    struct
      {
	int magic;
	FILE *stream;
      } glue;
    struct _iobuf
      {
	int _cnt;
	unsigned char *_ptr;
	unsigned char *_base;
	int _bufsiz;
	short int _flag;
	char _file;
      } unix_iobuf;
  } unix_FILE;

/* These are the Unix stdio's stdin, stdout, and stderr.  */
unix_FILE _iob[] =
  {
    { { _GLUEMAGIC, stdin, } },
    { { _GLUEMAGIC, stdout, } },
    { { _GLUEMAGIC, stderr, } },
  };

/* Called by the Unix stdio `getc' macro.  */
int
DEFUN(_filbuf, (file), unix_FILE *file)
{
  /* Compensate for getc's decrement.  */
  switch (++file->glue.magic)
    {
    case _GLUEMAGIC:
      return getc(file->glue.stream);

    case  _IOMAGIC:
      return getc((FILE *) file);

    default:
      errno = EINVAL;
      return EOF;
    }
}

/* Called by the Unix stdio `putc' macro.  */
int
DEFUN(_flsbuf, (c, file),
      int c AND unix_FILE *file)
{
  /* Compensate for putc's decrement.  */
  switch (++file->glue.magic)
    {
    case _GLUEMAGIC:
      return putc(c, file->glue.stream);

    case  _IOMAGIC:
      return putc(c, (FILE *) file);

    default:
      errno = EINVAL;
      return EOF;
    }
}
