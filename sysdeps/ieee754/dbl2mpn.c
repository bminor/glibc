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
#include <stdlib.h>

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

#if BITS_PER_MP_LIMB == 32
  res_ptr[0] = u.ieee.mantissa1; /* Low-order 32 bits of fraction.  */
  res_ptr[1] = u.ieee.mantissa0; /* High-order 20 bits.  */
  #define N 2
  #define IMPL1 1 << 20
#elif BITS_PER_MP_LIMB == 64
  /* Hopefully the compiler will combine the two bitfield extracts
     and this composition into just the original quadword extract.  */
  res_ptr[0] = (u.ieee.mantissa0 << 20) | u.ieee.mantissa1;
  #define N 1
#else
  #error "mp_limb size " BITS_PER_MP_LIMB "not accounted for"
#endif

  if (u.ieee.exponent == 0)
    {
      /* A biased exponent of zero is a special case.
	 Either it is a zero or it is a denormal number.  */
      if (res_ptr[0] == 0 && res_ptr[N - 1] == 0) /* Assumes N<=2.  */
	/* It's zero.  Just one limb records that.  */
	return 1;

      /* It is a denormal number, meaning it has no implicit leading
	 one bit, and its exponent is in fact the format minimum.  */
      *expt = -1021;
#if N == 2
      if (res_ptr[1] == 0)
	/* No point in using an extra high-order limb that is zero.  */
	return 1;
#endif
    }
  else
    /* Add the implicit leading one bit for a normalized number.  */
    res_ptr[N - 1] |= 1 << (52 - ((N - 1) * BITS_PER_MP_LIMB));

  return N;
}
