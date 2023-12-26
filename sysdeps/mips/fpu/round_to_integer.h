/* Round to integer generic implementation.
   Copyright (C) 2023 Free Software .
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _ROUND_TO_INTEGER_H
#define _ROUND_TO_INTEGER_H

#include <fenv_private.h>
#include <sysdeps/ieee754/dbl-64/math_config.h>

enum round_mode
{
  CEIL,
  FLOOR,
};

static inline double
round_to_integer_double (enum round_mode mode, double x)
{
  uint64_t hx = asuint64 (x);
  int ex = (hx & ~SIGN_MASK) >> MANTISSA_WIDTH;

  double r = x;

  fenv_t fe;
  libc_feholdexcept (&fe);
  switch (mode)
  {
  case CEIL:
    asm ("ceil.l.d %0, %0" : "+f" (r));
    break;
  case FLOOR:
    asm ("floor.l.d %0, %0" : "+f" (r));
    break;
  }
  libc_fesetenv (&fe);

  /* Inf or NaN.  */
  if (__glibc_unlikely (ex == 0x7ff))
    return x + x;

  /* Number input and no fraction, return the number itself.  */
  if (ex >= 1024 + 53)
    return x;

  asm ("cvt.d.l %0, %1\n" : "+f" (r));
  /* The copysign seems to generate better code.  */
#if 1
  return copysign (r, x);
#else
  bool sign = hx & SIGN_MASK;
  if (ex >= 1023)
    return r;
  return sign ? -r : r;
#endif
}

#endif
