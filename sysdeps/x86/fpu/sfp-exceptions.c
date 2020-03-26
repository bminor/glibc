/* x86_64 soft-fp exception handling for _Float128.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#include <fenv.h>
#include <float.h>
#include <math-barriers.h>
#include <soft-fp.h>

void
__sfp_handle_exceptions (int _fex)
{
  if (_fex & FP_EX_INVALID)
    {
      float f = 0.0f;
      math_force_eval (f / f);
    }
  if (_fex & FP_EX_DENORM)
    {
      float f = FLT_MIN, g = 2.0f;
      math_force_eval (f / g);
    }
  if (_fex & FP_EX_DIVZERO)
    {
      float f = 1.0f, g = 0.0f;
      math_force_eval (f / g);
    }
  if (_fex & FP_EX_OVERFLOW)
    {
      float force_underflow = FLT_MAX * FLT_MAX;
      math_force_eval (force_underflow);
    }
  if (_fex & FP_EX_UNDERFLOW)
    {
      float force_overflow = FLT_MIN * FLT_MIN;
      math_force_eval (force_overflow);
    }
  if (_fex & FP_EX_INEXACT)
    {
      float f = 1.0f, g = 3.0f;
      math_force_eval (f / g);
    }
}
