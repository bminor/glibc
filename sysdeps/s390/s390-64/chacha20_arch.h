/* s390x optimization for ChaCha20.VE_S390_VX_ASM_SUPPORT
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
#include <sys/auxv.h>

unsigned int __chacha20_s390x_vx_blocks8 (uint32_t *state, uint8_t *dst,
					  const uint8_t *src, size_t nblks)
     attribute_hidden;

static inline void
chacha20_crypt (uint32_t *state, uint8_t *dst, const uint8_t *src,
		size_t bytes)
{
#ifdef HAVE_S390_VX_ASM_SUPPORT
  _Static_assert (CHACHA20_BUFSIZE % 8 == 0,
		  "CHACHA20_BUFSIZE not multiple of 8");
  _Static_assert (CHACHA20_BUFSIZE >= CHACHA20_BLOCK_SIZE * 8,
		  "CHACHA20_BUFSIZE < CHACHA20_BLOCK_SIZE * 8");

  if (GLRO(dl_hwcap) & HWCAP_S390_VX)
    {
      __chacha20_s390x_vx_blocks8 (state, dst, src,
				   CHACHA20_BUFSIZE / CHACHA20_BLOCK_SIZE);
      return;
    }
#endif
  chacha20_crypt_generic (state, dst, src, bytes);
}
