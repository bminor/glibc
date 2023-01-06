/* Compute x * y + z as ternary operation.
   Copyright (C) 1997-2023 Free Software Foundation, Inc.
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

#define NO_MATH_REDIRECT
#define f64xfmaf128 __hide_f64xfmaf128
#include <math.h>
#undef f64xfmaf128
#include <libm-alias-ldouble.h>
#include <math-narrow-alias.h>

long double
__fmal (long double x, long double y, long double z)
{
  return (x * y) + z;
}
libm_alias_ldouble (__fma, fma)
libm_alias_ldouble_narrow (__fma, fma)
