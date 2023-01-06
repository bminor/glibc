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
#define dfmal __hide_dfmal
#define f32xfmaf64 __hide_f32xfmaf64
#include <math.h>
#undef dfmal
#undef f32xfmaf64
#include <libm-alias-double.h>
#include <math-narrow-alias.h>

double
__fma (double x, double y, double z)
{
  return (x * y) + z;
}
#ifndef __fma
libm_alias_double (__fma, fma)
libm_alias_double_narrow (__fma, fma)
#endif
