/* Generic ChaCha20 implementation (used on arc4random).
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

#include <array_length.h>
#include <endian.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* 32-bit stream position, then 96-bit nonce.  */
#define CHACHA20_IV_SIZE	16
#define CHACHA20_KEY_SIZE	32

#define CHACHA20_STATE_LEN	16

/* The ChaCha20 implementation is based on RFC8439 [1], omitting the final
   XOR of the keystream with the plaintext because the plaintext is a
   stream of zeros.  */

enum chacha20_constants
{
  CHACHA20_CONSTANT_EXPA = 0x61707865U,
  CHACHA20_CONSTANT_ND_3 = 0x3320646eU,
  CHACHA20_CONSTANT_2_BY = 0x79622d32U,
  CHACHA20_CONSTANT_TE_K = 0x6b206574U
};

static inline uint32_t
read_unaligned_32 (const uint8_t *p)
{
  uint32_t r;
  memcpy (&r, p, sizeof (r));
  return r;
}

static inline void
write_unaligned_32 (uint8_t *p, uint32_t v)
{
  memcpy (p, &v, sizeof (v));
}

#if __BYTE_ORDER == __BIG_ENDIAN
# define read_unaligned_le32(p) __builtin_bswap32 (read_unaligned_32 (p))
# define set_state(v)		__builtin_bswap32 ((v))
#else
# define read_unaligned_le32(p) read_unaligned_32 ((p))
# define set_state(v)		(v)
#endif

static inline void
chacha20_init (uint32_t *state, const uint8_t *key, const uint8_t *iv)
{
  state[0]  = CHACHA20_CONSTANT_EXPA;
  state[1]  = CHACHA20_CONSTANT_ND_3;
  state[2]  = CHACHA20_CONSTANT_2_BY;
  state[3]  = CHACHA20_CONSTANT_TE_K;

  state[4]  = read_unaligned_le32 (key + 0 * sizeof (uint32_t));
  state[5]  = read_unaligned_le32 (key + 1 * sizeof (uint32_t));
  state[6]  = read_unaligned_le32 (key + 2 * sizeof (uint32_t));
  state[7]  = read_unaligned_le32 (key + 3 * sizeof (uint32_t));
  state[8]  = read_unaligned_le32 (key + 4 * sizeof (uint32_t));
  state[9]  = read_unaligned_le32 (key + 5 * sizeof (uint32_t));
  state[10] = read_unaligned_le32 (key + 6 * sizeof (uint32_t));
  state[11] = read_unaligned_le32 (key + 7 * sizeof (uint32_t));

  state[12] = read_unaligned_le32 (iv + 0 * sizeof (uint32_t));
  state[13] = read_unaligned_le32 (iv + 1 * sizeof (uint32_t));
  state[14] = read_unaligned_le32 (iv + 2 * sizeof (uint32_t));
  state[15] = read_unaligned_le32 (iv + 3 * sizeof (uint32_t));
}

static inline uint32_t
rotl32 (unsigned int shift, uint32_t word)
{
  return (word << (shift & 31)) | (word >> ((-shift) & 31));
}

static void
state_final (const uint8_t *src, uint8_t *dst, uint32_t v)
{
#ifdef CHACHA20_XOR_FINAL
  v ^= read_unaligned_32 (src);
#endif
  write_unaligned_32 (dst, v);
}

static inline void
chacha20_block (uint32_t *state, uint8_t *dst, const uint8_t *src)
{
  uint32_t x0, x1, x2, x3, x4, x5, x6, x7;
  uint32_t x8, x9, x10, x11, x12, x13, x14, x15;

  x0 = state[0];
  x1 = state[1];
  x2 = state[2];
  x3 = state[3];
  x4 = state[4];
  x5 = state[5];
  x6 = state[6];
  x7 = state[7];
  x8 = state[8];
  x9 = state[9];
  x10 = state[10];
  x11 = state[11];
  x12 = state[12];
  x13 = state[13];
  x14 = state[14];
  x15 = state[15];

  for (int i = 0; i < 20; i += 2)
    {
#define QROUND(_x0, _x1, _x2, _x3) 			\
  do {							\
   _x0 = _x0 + _x1; _x3 = rotl32 (16, (_x0 ^ _x3)); 	\
   _x2 = _x2 + _x3; _x1 = rotl32 (12, (_x1 ^ _x2)); 	\
   _x0 = _x0 + _x1; _x3 = rotl32 (8,  (_x0 ^ _x3));	\
   _x2 = _x2 + _x3; _x1 = rotl32 (7,  (_x1 ^ _x2));	\
  } while(0)

      QROUND (x0, x4, x8,  x12);
      QROUND (x1, x5, x9,  x13);
      QROUND (x2, x6, x10, x14);
      QROUND (x3, x7, x11, x15);

      QROUND (x0, x5, x10, x15);
      QROUND (x1, x6, x11, x12);
      QROUND (x2, x7, x8,  x13);
      QROUND (x3, x4, x9,  x14);
    }

  state_final (&src[0], &dst[0], set_state (x0 + state[0]));
  state_final (&src[4], &dst[4], set_state (x1 + state[1]));
  state_final (&src[8], &dst[8], set_state (x2 + state[2]));
  state_final (&src[12], &dst[12], set_state (x3 + state[3]));
  state_final (&src[16], &dst[16], set_state (x4 + state[4]));
  state_final (&src[20], &dst[20], set_state (x5 + state[5]));
  state_final (&src[24], &dst[24], set_state (x6 + state[6]));
  state_final (&src[28], &dst[28], set_state (x7 + state[7]));
  state_final (&src[32], &dst[32], set_state (x8 + state[8]));
  state_final (&src[36], &dst[36], set_state (x9 + state[9]));
  state_final (&src[40], &dst[40], set_state (x10 + state[10]));
  state_final (&src[44], &dst[44], set_state (x11 + state[11]));
  state_final (&src[48], &dst[48], set_state (x12 + state[12]));
  state_final (&src[52], &dst[52], set_state (x13 + state[13]));
  state_final (&src[56], &dst[56], set_state (x14 + state[14]));
  state_final (&src[60], &dst[60], set_state (x15 + state[15]));

  state[12]++;
}

static void
__attribute_maybe_unused__
chacha20_crypt_generic (uint32_t *state, uint8_t *dst, const uint8_t *src,
			size_t bytes)
{
  while (bytes >= CHACHA20_BLOCK_SIZE)
    {
      chacha20_block (state, dst, src);

      bytes -= CHACHA20_BLOCK_SIZE;
      dst += CHACHA20_BLOCK_SIZE;
      src += CHACHA20_BLOCK_SIZE;
    }

  if (__glibc_unlikely (bytes != 0))
    {
      uint8_t stream[CHACHA20_BLOCK_SIZE];
      chacha20_block (state, stream, src);
      memcpy (dst, stream, bytes);
      explicit_bzero (stream, sizeof stream);
    }
}

/* Get the architecture optimized version.  */
#include <chacha20_arch.h>
