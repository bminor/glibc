/* Double-precision math error handling.
   Copyright (C) 2019 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include "math_config_dbl.h"
/* NOINLINE prevents fenv semantics breaking optimizations.  */
NOINLINE static double
xflow (uint32_t sign, double y)
{
  y = (sign ? -y : y) * y;
  return y;
}

attribute_hidden double
__math_uflow (uint32_t sign)
{
  return xflow (sign, 0x1p-767);
}

attribute_hidden double
__math_oflow (uint32_t sign)
{
  return xflow (sign, 0x1p769);
}


attribute_hidden double
__math_invalid(double x)
{
  return (x - x) / (x - x);
}

attribute_hidden double
__math_divzero(uint32_t sign)
{
  return (double)(sign ? -1.0 : 1.0) / 0.0;
}
