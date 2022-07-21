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

#include <isa-level.h>
#include <ldsodefs.h>
#include <cpu-features.h>
#include <sys/param.h>

unsigned int __chacha20_sse2_blocks4 (uint32_t *state, uint8_t *dst,
				      const uint8_t *src, size_t nblks)
     attribute_hidden;
unsigned int __chacha20_avx2_blocks8 (uint32_t *state, uint8_t *dst,
				      const uint8_t *src, size_t nblks)
     attribute_hidden;

static inline void
chacha20_crypt (uint32_t *state, uint8_t *dst, const uint8_t *src,
		size_t bytes)
{
  _Static_assert (CHACHA20_BUFSIZE % 4 == 0 && CHACHA20_BUFSIZE % 8 == 0,
		  "CHACHA20_BUFSIZE not multiple of 4 or 8");
  _Static_assert (CHACHA20_BUFSIZE >= CHACHA20_BLOCK_SIZE * 8,
		  "CHACHA20_BUFSIZE < CHACHA20_BLOCK_SIZE * 8");

#if MINIMUM_X86_ISA_LEVEL > 2
  __chacha20_avx2_blocks8 (state, dst, src,
			   CHACHA20_BUFSIZE / CHACHA20_BLOCK_SIZE);
#else
  const struct cpu_features* cpu_features = __get_cpu_features ();

  /* AVX2 version uses vzeroupper, so disable it if RTM is enabled.  */
  if (X86_ISA_CPU_FEATURE_USABLE_P (cpu_features, AVX2)
      && X86_ISA_CPU_FEATURES_ARCH_P (cpu_features, Prefer_No_VZEROUPPER, !))
    __chacha20_avx2_blocks8 (state, dst, src,
			     CHACHA20_BUFSIZE / CHACHA20_BLOCK_SIZE);
  else
    __chacha20_sse2_blocks4 (state, dst, src,
			     CHACHA20_BUFSIZE / CHACHA20_BLOCK_SIZE);
#endif
}
