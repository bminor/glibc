/* Chacha20 implementation, used on arc4random.
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

#include <ldsodefs.h>
#include <cpu-features.h>
#include <sys/param.h>

unsigned int __chacha20_sse2_blocks4 (uint32_t *state, uint8_t *dst,
				      const uint8_t *src, size_t nblks)
     attribute_hidden;

static inline void
chacha20_crypt (uint32_t *state, uint8_t *dst, const uint8_t *src,
		size_t bytes)
{
  _Static_assert (CHACHA20_BUFSIZE % 4 == 0,
		  "CHACHA20_BUFSIZE not multiple of 4");
  _Static_assert (CHACHA20_BUFSIZE >= CHACHA20_BLOCK_SIZE * 4,
		  "CHACHA20_BUFSIZE <= CHACHA20_BLOCK_SIZE * 4");

  __chacha20_sse2_blocks4 (state, dst, src,
			   CHACHA20_BUFSIZE / CHACHA20_BLOCK_SIZE);
}
