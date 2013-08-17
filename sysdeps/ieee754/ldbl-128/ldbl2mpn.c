/* Copyright (C) 1995-2013 Free Software Foundation, Inc.
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

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"
#include <ieee754.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

/* Convert a `long double' in IEEE854 quad-precision format to a
   multi-precision integer representing the significand scaled up by its
   number of bits (113 for long double) and an integral power of two
   (MPN frexpl). */

mp_size_t
__mpn_extract_long_double (mp_ptr res_ptr, mp_size_t size,
			   int *expt, int *zero_bits, int *is_neg,
			   long double value)
{
  union ieee854_long_double u;
  u.d = value;

  *is_neg = u.ieee.negative;
  *expt = (int) u.ieee.exponent - IEEE854_LONG_DOUBLE_BIAS;

#if BITS_PER_MP_LIMB == 32
  res_ptr[0] = u.ieee.mantissa3; /* Low-order 32 bits of fraction.  */
  res_ptr[1] = u.ieee.mantissa2;
  res_ptr[2] = u.ieee.mantissa1;
  res_ptr[3] = u.ieee.mantissa0; /* High-order 32 bits.  */
  #define N 4
#elif BITS_PER_MP_LIMB == 64
  /* Hopefully the compiler will combine the two bitfield extracts
     and this composition into just the original quadword extract.  */
  res_ptr[0] = ((mp_limb_t) u.ieee.mantissa2 << 32) | u.ieee.mantissa3;
  res_ptr[1] = ((mp_limb_t) u.ieee.mantissa0 << 32) | u.ieee.mantissa1;
  #define N 2
#else
  #error "mp_limb size " BITS_PER_MP_LIMB "not accounted for"
#endif

/* The format does not fill the last limb.  There are some zeros.  */
#define NUM_LEADING_ZEROS (N * BITS_PER_MP_LIMB - LDBL_MANT_DIG)
  *zero_bits = NUM_LEADING_ZEROS;

  if (u.ieee.exponent == 0)
    {
      /* A biased exponent of zero is a special case.
	 Either it is a zero or it is a denormal number.  */
      if (res_ptr[0] == 0 && res_ptr[1] == 0
          && res_ptr[N - 2] == 0 && res_ptr[N - 1] == 0) /* Assumes N<=4.  */
	{
	  /* It's zero.  */
	  *expt = 0;
	  return 1;
	}
      else
	{
          /* It is a denormal number, meaning it has no implicit leading
	     one bit, and its exponent is in fact the format minimum.  */
	  int cnt;
	  int exp = 1 - IEEE854_LONG_DOUBLE_BIAS;
	  int n = N;

#if N == 2
	  if (res_ptr[N - 1] != 0)
	    count_leading_zeros (cnt, res_ptr[N - 1]);
	  else
	    {
	      count_leading_zeros (cnt, res_ptr[0]);
	      exp -= BITS_PER_MP_LIMB;
	      n = 1;
	    }
#else
	  int j;

	  for (j = N - 1; j > 0; j--)
	    if (res_ptr[j] != 0)
	      break;

	  count_leading_zeros (cnt, res_ptr[j]);
	  exp -= (N - 1 - j) * BITS_PER_MP_LIMB;
	  n = j + 1;
#endif
	  *zero_bits = cnt;
	  *expt = exp + NUM_LEADING_ZEROS - cnt;
	  return n;
	}
    }
  else
    /* Add the implicit leading one bit for a normalized number.  */
    res_ptr[N - 1] |= (mp_limb_t) 1 << (LDBL_MANT_DIG - 1
					- ((N - 1) * BITS_PER_MP_LIMB));

  return N;
}
