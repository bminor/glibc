/* PowerPC optimization for ChaCha20.
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

#include <stdbool.h>
#include <ldsodefs.h>

unsigned int __chacha20_power8_blocks4 (uint32_t *state, uint8_t *dst,
					const uint8_t *src, size_t nblks)
     attribute_hidden;

static void
chacha20_crypt (uint32_t *state, uint8_t *dst,
		const uint8_t *src, size_t bytes)
{
  _Static_assert (CHACHA20_BUFSIZE % 4 == 0,
		  "CHACHA20_BUFSIZE not multiple of 4");
  _Static_assert (CHACHA20_BUFSIZE >= CHACHA20_BLOCK_SIZE * 4,
		  "CHACHA20_BUFSIZE < CHACHA20_BLOCK_SIZE * 4");

  unsigned long int hwcap = GLRO(dl_hwcap);
  unsigned long int hwcap2 = GLRO(dl_hwcap2);
  if (hwcap2 & PPC_FEATURE2_ARCH_2_07 && hwcap & PPC_FEATURE_HAS_ALTIVEC)
    __chacha20_power8_blocks4 (state, dst, src,
			       CHACHA20_BUFSIZE / CHACHA20_BLOCK_SIZE);
  else
    chacha20_crypt_generic (state, dst, src, bytes);
}
