/* Simple memchr.

   Copyright (C) 2022 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <string.h>

void *__memchr (const void *s, int c, size_t n)
{
  unsigned char uc = c;
  const unsigned char *us = s;
  for (; n > 0; n--, us++)
    if (*us == uc)
      return (void *) us;
  return NULL;
}

weak_alias (__memchr, memchr)
libc_hidden_builtin_def (memchr)
