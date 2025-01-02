/* Copyright (C) 1996-2025 Free Software Foundation, Inc.

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
#include <get-rounding-mode.h>
#include <stdint.h>
#include <math-barriers.h>
#include <libm-alias-double.h>

long int
__lrint (double x)
{
  double r =  __builtin_rint (x);

  /* Prevent gcc from calling lrint directly when compiled with
     -fno-math-errno by inserting a barrier.  */

  math_opt_barrier (r);
  return r;
}

libm_alias_double (__lrint, lrint)
