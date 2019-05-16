/* Copyright (C) 2005-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.org>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <libioP.h>

/* See libio/fwprintf.c.  */
static int
call_fwprintf (FILE *stream, unsigned int mode_flags,
	       const wchar_t *format, ...)
{
  va_list arg;
  va_start (arg, format);
  int done = __vfwprintf_internal (stream, format, arg, mode_flags);
  va_end (arg);
  return done;
}

static int
locked_vfxprintf (FILE *fp, const char *fmt, va_list ap,
		  unsigned int mode_flags)
{
  if (_IO_fwide (fp, 0) <= 0)
    return __vfprintf_internal (fp, fmt, ap, mode_flags);

  int saved_errno = errno;

  /* Format the narrow string as a multibyte string.  Try to use an
     on-stack buffer first, to avoid the heap allocation.  */
  char buffer[512];
  va_list ap1;
  va_copy (ap1, ap);
  int res = __vsnprintf_internal (buffer, sizeof (buffer),
				  fmt, ap1, mode_flags);
  va_end (ap1);
  if (res < 0)
    return res;
  char *ptr;
  if (res < sizeof (buffer))
    ptr = buffer;
  else
    {
      /* Use a heap allocation for a large buffer.  */
      if (res == INT_MAX)
	{
	  __set_errno (EOVERFLOW);
	  return -1;
	}
      size_t len = res + 1;
      ptr = malloc (len);
      if (ptr == NULL)
	return -1;
      __set_errno (saved_errno);
      res = __vsnprintf_internal (ptr, len, fmt, ap, mode_flags);
      if (res < 0)
	return -1;
    }

  /* Write the formatted multibyte string to the wide stream.  */
  if (res >= 0)
    res = call_fwprintf (fp, mode_flags, L"%s", ptr);

  if (ptr != buffer)
    free (ptr);
  __set_errno (saved_errno);

  return res;
}

int
__vfxprintf (FILE *fp, const char *fmt, va_list ap,
	     unsigned int mode_flags)
{
  if (fp == NULL)
    fp = stderr;
  _IO_flockfile (fp);
  int res = locked_vfxprintf (fp, fmt, ap, mode_flags);
  _IO_funlockfile (fp);
  return res;
}

int
__fxprintf (FILE *fp, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  int res = __vfxprintf (fp, fmt, ap, 0);
  va_end (ap);
  return res;
}

int
__fxprintf_nocancel (FILE *fp, const char *fmt, ...)
{
  if (fp == NULL)
    fp = stderr;

  va_list ap;
  va_start (ap, fmt);
  _IO_flockfile (fp);
  int save_flags2 = fp->_flags2;
  fp->_flags2 |= _IO_FLAGS2_NOTCANCEL;

  int res = locked_vfxprintf (fp, fmt, ap, 0);

  fp->_flags2 = save_flags2;
  _IO_funlockfile (fp);
  va_end (ap);
  return res;
}
