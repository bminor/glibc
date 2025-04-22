/* Return is number is Not a Number.
   Copyright (C) 2002-2025 Free Software Foundation, Inc.
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

#include <math.h>
#include <stdbit.h>
#include <math_private.h>

int
__isnanl (_Float128 x)
{
  uint64_t hx,lx;
  GET_LDOUBLE_WORDS64 (hx,lx,x);
  hx &= 0x7fffffffffffffffULL;
  /* NaN - exponent == 0x7fff and non-zero significand.  */
  return hx >> 48 == 0x7fff
    && ((hx & ~0xffff000000000000ULL) != 0 || lx != 0);
}
mathx_hidden_def (__isnanl)
weak_alias (__isnanl, isnanl)
