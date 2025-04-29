/* Get integer exponent of a floating-point value.
   Copyright (C) 2025 Free Software Foundation, Inc.
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

/* Use the double version of EDOM/invalid operation handling.  */
#include "math_config.h"

static RET_TYPE
llogb_nan_inf_subnormal (uint64_t hx, uint64_t lx)
{
  if (hx <= 0x0001000000000000ULL)
    {
      /* Zero or subnormal.  */
      if ((hx | lx) == 0)
	return RET_INVALID (RET_LOGB0);
      /* Subnormal */
      if (hx == 0)
	return -16431L - stdc_leading_zeros (lx);
      else
	return -16382L - stdc_leading_zeros (hx << 15);
    }
  return RET_INVALID (RET_LOGBNAN);
}

static inline RET_TYPE
IMPL_NAME (FLOAT x)
{
  uint64_t hx, lx;
  GET_FLOAT128_WORDS64 (hx, lx, x);
  hx &= 0x7fffffffffffffffULL;

#if _GL_HAS_BUILTIN_ILOGB
  /* Check for exceptional cases.  */
  if (__glibc_likely (! M_SUF(__builtin_test_dc_ilogb) (x, 0x7f)))
    return M_SUF (__builtin_ilogb) (x);
#else
  if (__glibc_likely (hx < 0x7fff000000000000ULL
		      && hx > 0x0001000000000000ULL))
    return (hx >> 48) - 0x3fff;
#endif

  return llogb_nan_inf_subnormal (hx, lx);
}
