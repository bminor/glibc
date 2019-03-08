/* Round to integer generic implementation.
   Copyright (C) 2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef _ROUND_TO_INTEGER_H
#define _ROUND_TO_INTEGER_H

#include <fenv_private.h>

enum round_mode
{
  CEIL,
  FLOOR
};

static inline void
set_fenv_mode (enum round_mode mode)
{
  int rmode;
  switch (mode)
  {
  case CEIL:  rmode = FE_UPWARD; break;
  case FLOOR: rmode = FE_DOWNWARD; break;
  default:    rmode = FE_TONEAREST; break;
  }
  __fesetround_inline_nocheck (rmode);
}

static inline float
round_to_integer_float (enum round_mode mode, float x)
{
  /* Ensure sNaN input is converted to qNaN.  */
  if (__glibc_unlikely (isnan (x)))
    return x + x;

  if (fabs (x) > 0x1p+23)
    return x;

  float r = x;

  /* Save current FPU rounding mode and inexact state.  */
  fenv_t fe = fegetenv_register ();
  set_fenv_mode (mode);
  if (x > 0.0)
    {
      r += 0x1p+23;
      r -= 0x1p+23;
      r = fabs (r);
    }
  else if (x < 0.0)
    {
      r -= 0x1p+23;
      r += 0x1p+23;
      r = -fabs (r);
    }
  __builtin_mtfsf (0xff, fe);

  return r;
}

static inline double
round_to_integer_double (enum round_mode mode, double x)
{
  /* Ensure sNaN input is converted to qNaN.  */
  if (__glibc_unlikely (isnan (x)))
    return x + x;

  if (fabs (x) > 0x1p+52)
    return x;

  double r = x;

  /* Save current FPU rounding mode and inexact state.  */
  fenv_t fe = fegetenv_register ();
  set_fenv_mode (mode);
  if (x > 0.0)
    {
      r += 0x1p+52;
      r -= 0x1p+52;
      r = fabs (r);
    }
  else if (x < 0.0)
    {
      r -= 0x1p+52;
      r += 0x1p+52;
      r = -fabs (r);
    }
  __builtin_mtfsf (0xff, fe);

  return r;
}

#endif
