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

/* Convert a `long double' in IBM extended format to a multi-precision
   integer representing the significand scaled up by its number of
   bits (106 for long double) and an integral power of two (MPN
   frexpl). */

mp_size_t
__mpn_extract_long_double (mp_ptr res_ptr, mp_size_t size,
			   int *expt, int *zero_bits, int *is_neg,
			   long double value)
{
  union ibm_extended_long_double u;
  uint64_t hi, lo;
  int ediff;

  u.ld = value;

  *is_neg = u.d[0].ieee.negative;
  *expt = (int) u.d[0].ieee.exponent - IEEE754_DOUBLE_BIAS;
#define NUM_LEADING_ZEROS (128 - (LDBL_MANT_DIG + 11))
  *zero_bits = NUM_LEADING_ZEROS;

  lo = ((uint64_t) u.d[1].ieee.mantissa0 << 32) | u.d[1].ieee.mantissa1;
  hi = ((uint64_t) u.d[0].ieee.mantissa0 << 32) | u.d[0].ieee.mantissa1;

  if (u.d[0].ieee.exponent != 0)
    {
      /* If the high double is not a denormal or zero then set the hidden
	 53rd bit.  */
      hi |= (uint64_t) 1 << 52;

      /* If the lower double is not a denormal or zero then set the hidden
	 53rd bit.  */
      if (u.d[1].ieee.exponent != 0)
	lo |= (uint64_t) 1 << 52;
      else
	lo = lo << 1;

      /* We currently only have 53 bits in lo.  Gain a few more bits
	 of precision.  */
      lo = lo << 11;

      /* The lower double is normalized separately from the upper.  We may
	 need to adjust the lower manitissa to reflect this.  */
      ediff = u.d[0].ieee.exponent - u.d[1].ieee.exponent - 53;
      if (ediff > 0)
	{
	  if (ediff < 64)
	    lo = lo >> ediff;
	  else
	    lo = 0;
	}
      else if (ediff < 0)
	lo = lo << -ediff;

      /* The high double may be rounded and the low double reflects the
	 difference between the long double and the rounded high double
	 value.  This is indicated by a differnce between the signs of the
	 high and low doubles.  */
      if (u.d[0].ieee.negative != u.d[1].ieee.negative
	  && lo != 0)
	{
	  hi--;
	  lo = -lo;
	  if (hi < (uint64_t) 1 << 52)
	    {
	      /* We have a borrow from the hidden bit, so shift left 1.  */
	      hi = (hi << 1) | (lo >> 63);
	      lo = lo << 1;
	      (*expt)--;
	    }
	}

#if BITS_PER_MP_LIMB == 32
      res_ptr[0] = lo;
      res_ptr[1] = lo >> 32;
      res_ptr[2] = hi;
      res_ptr[3] = hi >> 32;
      return 4;
#elif BITS_PER_MP_LIMB == 64
      res_ptr[0] = lo;
      res_ptr[1] = hi;
      return 2;
#else
# error "mp_limb size " BITS_PER_MP_LIMB "not accounted for"
#endif
    }

  /* The high double is a denormal or zero.  The low double must
     be zero.  A denormal is interpreted as having a biased
     exponent of 1.  */
  res_ptr[0] = hi;
#if BITS_PER_MP_LIMB == 32
  res_ptr[1] = hi >> 32;
#endif
  if (hi == 0)
    {
      /* It's zero.  */
      *expt = 0;
      return 1;
    }
  else
    {
      int cnt;
      int exp = 1 - IEEE754_DOUBLE_BIAS;
      int n = 1;

#if BITS_PER_MP_LIMB == 32
      if (res_ptr[1] != 0)
	{
	  count_leading_zeros (cnt, res_ptr[1]);
	  n = 2;
	}
      else
	{
	  count_leading_zeros (cnt, res_ptr[0]);
	  exp -= BITS_PER_MP_LIMB;
	}
#else
      count_leading_zeros (cnt, hi);
#endif
      *zero_bits = cnt;
      *expt = exp + NUM_LEADING_ZEROS - cnt;
      return n;
    }
}
