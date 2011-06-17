/* Pythagorean addition using floats
   Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Adhemerval Zanella <azanella@br.ibm.com>, 2011

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "math.h"

/* __ieee754_hypotf(x,y)
 *
 * This a FP only version without any FP->INT conversion.
 * It is similar to default C version, making appropriates
 * overflow and underflows checks as well scaling when it
 * is needed.
 */

static const float two30  = 1.0737418e09;
static const float two50  = 1.1259000e15;
static const float two60  = 1.1529221e18;
static const float two126 = 8.5070592e+37;
static const float twoM50 = 8.8817842e-16;
static const float twoM60 = 6.7762644e-21;
static const float pdnum  = 1.1754939e-38;

float
__ieee754_hypotf (float x, float y)
{
  float j;

  if (isinff(x) || isinff(y))
    {
      return INFINITY;
    }
  if (isnanf(x) || isnanf(y))
    {
      return NAN;
    }
  x = __builtin_fabsf (x);
  y = __builtin_fabsf (y);
  if (y > x)
    {
      j = x;
      x = y;
      y = j;
    }
  if (y == 0.0 || (x / y) > two30)
    {
      return x + y;
    }
  if (x > two50)
    {
      x *= twoM60;
      y *= twoM60;
      return __builtin_sqrtf (x * x + y * y) / twoM60;
    }
  if (y < twoM50)
    {
      if (y <= pdnum)
	{
	  x *= two126;
	  y *= two126;
	  return __builtin_sqrtf (x * x + y * y) / two126;
	}
      else
	{
	  x *= two60;
	  y *= two60;
	  return __builtin_sqrtf (x * x + y * y) / two60;
	}
    }
  return __builtin_sqrtf (x * x + y * y);
}
