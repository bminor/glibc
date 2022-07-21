/* Random pseudo generator number which returns a single 32 bit value
   uniformly distributed but with an upper_bound.
   Copyright (C) 2022 Free Software Foundation, Inc.
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

#include <endian.h>
#include <libc-lock.h>
#include <stdlib.h>
#include <sys/param.h>

/* Return the number of bytes which cover values up to the limit.  */
__attribute__ ((const))
static uint32_t
byte_count (uint32_t n)
{
  if (n < (1U << 8))
    return 1;
  else if (n < (1U << 16))
    return 2;
  else if (n < (1U << 24))
    return 3;
  else
    return 4;
}

/* Fill the lower bits of the result with randomness, according to the
   number of bytes requested.  */
static void
random_bytes (uint32_t *result, uint32_t byte_count)
{
  *result = 0;
  unsigned char *ptr = (unsigned char *) result;
  if (__BYTE_ORDER == __BIG_ENDIAN)
    ptr += 4 - byte_count;
  __arc4random_buf (ptr, byte_count);
}

uint32_t
__arc4random_uniform (uint32_t n)
{
  if (n <= 1)
    /* There is no valid return value for a zero limit, and 0 is the
       only possible result for limit 1.  */
    return 0;

  /* The bits variable serves as a source for bits.  Prefetch the
     minimum number of bytes needed.  */
  uint32_t count = byte_count (n);
  uint32_t bits_length = count * CHAR_BIT;
  uint32_t bits;
  random_bytes (&bits, count);

  /* Powers of two are easy.  */
  if (powerof2 (n))
    return bits & (n - 1);

  /* The general case.  This algorithm follows Jérémie Lumbroso,
     Optimal Discrete Uniform Generation from Coin Flips, and
     Applications (2013), who credits Donald E. Knuth and Andrew
     C. Yao, The complexity of nonuniform random number generation
     (1976), for solving the general case.

     The implementation below unrolls the initialization stage of the
     loop, where v is less than n.  */

  /* Use 64-bit variables even though the intermediate results are
     never larger than 33 bits.  This ensures the code is easier to
     compile on 64-bit architectures.  */
  uint64_t v;
  uint64_t c;

  /* Initialize v and c.  v is the smallest power of 2 which is larger
     than n.*/
  {
    uint32_t log2p1 = 32 - __builtin_clz (n);
    v = 1ULL << log2p1;
    c = bits & (v - 1);
    bits >>= log2p1;
    bits_length -= log2p1;
  }

  /* At the start of the loop, c is uniformly distributed within the
     half-open interval [0, v), and v < 2n < 2**33.  */
  while (true)
    {
      if (v >= n)
        {
          /* If the candidate is less than n, accept it.  */
          if (c < n)
            /* c is uniformly distributed on [0, n).  */
            return c;
          else
            {
              /* c is uniformly distributed on [n, v).  */
              v -= n;
              c -= n;
              /* The distribution was shifted, so c is uniformly
                 distributed on [0, v) again.  */
            }
        }
      /* v < n here.  */

      /* Replenish the bit source if necessary.  */
      if (bits_length == 0)
        {
          /* Overwrite the least significant byte.  */
	  random_bytes (&bits, 1);
	  bits_length = CHAR_BIT;
        }

      /* Double the range.  No overflow because v < n < 2**32.  */
      v *= 2;
      /* v < 2n here.  */

      /* Extract a bit and append it to c.  c remains less than v and
         thus 2**33.  */
      c = (c << 1) | (bits & 1);
      bits >>= 1;
      --bits_length;

      /* At this point, c is uniformly distributed on [0, v) again,
         and v < 2n < 2**33.  */
    }
}
libc_hidden_def (__arc4random_uniform)
weak_alias (__arc4random_uniform, arc4random_uniform)
