/* Simple memcmp.

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

int memcmp (const void *s1, const void *s2, size_t n)
{
  const unsigned char *us1 = s1;
  const unsigned char *us2 = s2;
  for (; n > 0; us1++, us2++, n--)
    if (*us1 != *us2)
      return *us1 - *us2;
  return 0;
}

#undef bcmp
weak_alias (memcmp, bcmp)
libc_hidden_builtin_def (memcmp)

#undef __memcmpeq
strong_alias (memcmp, __memcmpeq)
libc_hidden_def(__memcmpeq)
