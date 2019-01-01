/* Measure stpcpy functions.
   Copyright (C) 2013-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#define STRCPY_RESULT(dst, len) ((dst) + (len))
#define TEST_MAIN
#ifndef WIDE
# define TEST_NAME "stpcpy"
#else
# define TEST_NAME "wcpcpy"
#endif /* WIDE */
#include "bench-string.h"
#ifndef WIDE
# define SIMPLE_STPCPY simple_stpcpy
#else
# define SIMPLE_STPCPY simple_wcpcpy
#endif /* WIDE */

CHAR *SIMPLE_STPCPY (CHAR *, const CHAR *);

IMPL (SIMPLE_STPCPY, 0)
IMPL (STPCPY, 1)

CHAR *
SIMPLE_STPCPY (CHAR *dst, const CHAR *src)
{
  while ((*dst++ = *src++) != '\0');
  return dst - 1;
}

#include "bench-strcpy.c"
