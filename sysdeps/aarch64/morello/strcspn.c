/* Simple strcspn.

   Copyright (C) 2022 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <string.h>

size_t strcspn (const char *str, const char *reject)
{
  if (reject[0] == 0 || reject[1] == 0)
    return __strchrnul (str, reject[0]) - str;

  unsigned char table[256] = {0};
  unsigned char *us;
  for (us = (unsigned char*) reject; *us != 0; table[*us++] = 1);
  table[*us] = 1;

  for (us = (unsigned char*) str; table[*us] == 0; us++);
  return us - (unsigned char *) str;
}
libc_hidden_builtin_def (strcspn)
