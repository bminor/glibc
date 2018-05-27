/* Shared module to test for relocation over with IFUNC symbols.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

#include <stddef.h>

void *
my_memmove(void *dst_p, const void *src_p, size_t n)
{
  const char *src = src_p;
  char *dst = dst_p;
  char *ret = dst;
  if (src < dst)
    {
      dst += n;
      src += n;
      while (n--)
	*--dst = *--src;
    }
  else
    while (n--)
      *dst++ = *src++;
  return ret;
}

void *memmove (void *, const void *, size_t)
  __attribute__ ((ifunc ("resolve_memmove")));

typedef void *(*memmove_t) (void *, const void *, size_t);

static memmove_t
resolve_memmove (void)
{
  return my_memmove;
}
