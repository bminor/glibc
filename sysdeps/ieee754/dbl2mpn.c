/* Copyright (C) 1993, 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include "gmp.h"
#include "gmp-impl.h"
#include "ieee754.h"

/* Convert a `double' in IEEE754 standard double-precision format to a
   multi-precision integer representing the significand scaled up by its
   number of bits (52 for double) and an integral power of two (MPN frexp). */

mp_size_t
__mpn_extract_double (mp_ptr res_ptr, mp_size_t size,
		      int *expt, int *is_neg,
		      double value)
{
  union ieee754_double u;
  u.d = value;

  *is_neg = u.ieee.negative;
  *expt = (int) u.ieee.exponent - 1022;

  res_ptr[0] = u.ieee.mantissa1; /* Low-order 32 bits of fraction.  */
  res_ptr[1] = u.ieee.mantissa0; /* High-order 20 bits.  */

  if (*expt == 0)
    {
      /* A biased exponent of zero is a special case.
	 Either it is a zero or it is a denormal number.  */
      if (res_ptr[0] == 0 && res_ptr[1] == 0)
	/* It's zero.  Just one limb records that.  */
	return 1;
      else
	/* It is a denormal number, meaning it has no implicit leading
	   one bit, and its exponent is in fact the format minimum.  */
	  *expt = -1022;
    }
  else
    /* Add the implicit leading one bit for a normalized number.  */
    res_ptr[1] |= 1 << 20;

  return 2;
}
