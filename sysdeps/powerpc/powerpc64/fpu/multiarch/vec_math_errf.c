/* Single-precision vector math error handling.
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

#include "math_config_flt.h"
/* NOINLINE prevents fenv semantics breaking optimizations.  */
NOINLINE static float
xflowf (uint32_t sign, float y)
{
  y = (sign ? -y : y) * y;
  return y;
}

attribute_hidden float
__math_uflowf (uint32_t sign)
{
  return xflowf (sign, 0x1p-95f);
}

attribute_hidden float
__math_oflowf (uint32_t sign)
{
  return xflowf (sign, 0x1p97f);
}
