/* ffsll -- find first set bit in a word, counted from least significant end.
   For AMD x86-64.
   This file is part of the GNU C Library.
   Copyright (C) 1991-2024 Free Software Foundation, Inc.

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

#define ffsl __something_else
#include <string.h>

#undef	ffsll

int
ffsll (long long int x)
{
  long long int cnt;

  asm ("mov $-1,%k0\n"	/* Initialize cnt to -1.  */
       "bsf %1,%0\n"	/* Count low bits in x and store in cnt.  */
       "inc %k0\n"	/* Increment cnt by 1.  */
       : "=&r" (cnt) : "r" (x));

  return cnt;
}

#ifndef __ILP32__
#undef	ffsl
weak_alias (ffsll, ffsl)
#endif
