/* Optimization of repeated integer division.
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
   <https://www.gnu.org/licenses/>.  */

#include <stdint.h>
#include <sys/param.h>

/* Precompute *MULTIPLIER for dividing by DIVISOR, which must be two
   or larger, and return the shift count (non-negative and less than
   32), for use with divopt_32 below.  */
static int __attribute__ ((used))
precompute_divopt_32 (uint32_t divisor, uint32_t *multiplier)
{
  if (divisor == 1)
    {
      *multiplier = 1;
      return 0;
    }

  int log2 = 32 - __builtin_clz (divisor);

  /* Handle powers-of-two first, so that we do not need to deal with
     the clz corner cases below.  */
  if (powerof2 (divisor))
    {
      *multiplier = 1;
      return log2 - 2;
    }

  if (log2 != 32)
    {
      /* Compute ceil (2**(32 + log2) / divisor).  The
         most-significant bit is always set and is discarded.  */
      *multiplier = (((uint64_t) 1 << (32 + log2)) + divisor) / divisor;
      return log2 - 1;
    }
  else
    {
      /* Perform a long division of 2**64 + (divisor - 1) by the
         divisor, encoded in base-2**32, using a 64-by-32 division.
         Start out with the first two digits, which are (1, 0).  2**32
         divided by the divisor is 1 because the divisor is larger
         than 2**31.  This set bit is discarded.  */
      uint64_t remainder = -divisor;

      /* Combine the remainder of the first division with the third
         and final base 2**32 digit.  */
      *multiplier = ((remainder << 32) | (divisor - 1)) / divisor;
      return 31;
    }
}

/* Return the quotient of DIVIDEND devided by the divisor that was
   used to compute MULTIPLIER and SHIFT via precompute_divopt_32.  */
static inline uint32_t
divopt_32 (uint32_t dividend, uint32_t multiplier, int shift)
{
  /* Approximation to the quotient.  */
  uint32_t quotient = ((uint64_t) dividend * multiplier) >> 32;
  /* Compute (dividend + quotient) / 2 without overflow.  */
  uint32_t temp = ((dividend - quotient) >> 1) + quotient;
  /* The result is in the higher-order bits.  */
  return temp >> shift;
}
