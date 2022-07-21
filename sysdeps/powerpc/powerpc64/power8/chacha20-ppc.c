/* Optimized PowerPC implementation of ChaCha20 cipher.
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

/* chacha20-ppc.c - PowerPC vector implementation of ChaCha20
   Copyright (C) 2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>

   This file is part of Libgcrypt.

   Libgcrypt is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   Libgcrypt is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <altivec.h>
#include <endian.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

typedef vector unsigned char vector16x_u8;
typedef vector unsigned int vector4x_u32;
typedef vector unsigned long long vector2x_u64;

#if __BYTE_ORDER == __BIG_ENDIAN
static const vector16x_u8 le_bswap_const =
  { 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12 };
#endif

static inline vector4x_u32
vec_rol_elems (vector4x_u32 v, unsigned int idx)
{
#if __BYTE_ORDER != __BIG_ENDIAN
  return vec_sld (v, v, (16 - (4 * idx)) & 15);
#else
  return vec_sld (v, v, (4 * idx) & 15);
#endif
}

static inline vector4x_u32
vec_load_le (unsigned long offset, const unsigned char *ptr)
{
  vector4x_u32 vec;
  vec = vec_vsx_ld (offset, (const uint32_t *)ptr);
#if __BYTE_ORDER == __BIG_ENDIAN
  vec = (vector4x_u32) vec_perm ((vector16x_u8)vec, (vector16x_u8)vec,
				 le_bswap_const);
#endif
  return vec;
}

static inline void
vec_store_le (vector4x_u32 vec, unsigned long offset, unsigned char *ptr)
{
#if __BYTE_ORDER == __BIG_ENDIAN
  vec = (vector4x_u32)vec_perm((vector16x_u8)vec, (vector16x_u8)vec,
			       le_bswap_const);
#endif
  vec_vsx_st (vec, offset, (uint32_t *)ptr);
}


static inline vector4x_u32
vec_add_ctr_u64 (vector4x_u32 v, vector4x_u32 a)
{
#if __BYTE_ORDER == __BIG_ENDIAN
  static const vector16x_u8 swap32 =
    { 4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11 };
  vector2x_u64 vec, add, sum;

  vec = (vector2x_u64)vec_perm ((vector16x_u8)v, (vector16x_u8)v, swap32);
  add = (vector2x_u64)vec_perm ((vector16x_u8)a, (vector16x_u8)a, swap32);
  sum = vec + add;
  return (vector4x_u32)vec_perm ((vector16x_u8)sum, (vector16x_u8)sum, swap32);
#else
  return (vector4x_u32)((vector2x_u64)(v) + (vector2x_u64)(a));
#endif
}

/**********************************************************************
  4-way chacha20
 **********************************************************************/

#define ROTATE(v1,rolv)			\
	__asm__ ("vrlw %0,%1,%2\n\t" : "=v" (v1) : "v" (v1), "v" (rolv))

#define PLUS(ds,s) \
	((ds) += (s))

#define XOR(ds,s) \
	((ds) ^= (s))

#define ADD_U64(v,a) \
	(v = vec_add_ctr_u64(v, a))

/* 4x4 32-bit integer matrix transpose */
#define transpose_4x4(x0, x1, x2, x3) ({ \
	vector4x_u32 t1 = vec_mergeh(x0, x2); \
	vector4x_u32 t2 = vec_mergel(x0, x2); \
	vector4x_u32 t3 = vec_mergeh(x1, x3); \
	x3 = vec_mergel(x1, x3); \
	x0 = vec_mergeh(t1, t3); \
	x1 = vec_mergel(t1, t3); \
	x2 = vec_mergeh(t2, x3); \
	x3 = vec_mergel(t2, x3); \
      })

#define QUARTERROUND2(a1,b1,c1,d1,a2,b2,c2,d2)			\
	PLUS(a1,b1); PLUS(a2,b2); XOR(d1,a1); XOR(d2,a2);	\
	    ROTATE(d1, rotate_16); ROTATE(d2, rotate_16);	\
	PLUS(c1,d1); PLUS(c2,d2); XOR(b1,c1); XOR(b2,c2);	\
	    ROTATE(b1, rotate_12); ROTATE(b2, rotate_12);	\
	PLUS(a1,b1); PLUS(a2,b2); XOR(d1,a1); XOR(d2,a2);	\
	    ROTATE(d1, rotate_8); ROTATE(d2, rotate_8);		\
	PLUS(c1,d1); PLUS(c2,d2); XOR(b1,c1); XOR(b2,c2);	\
	    ROTATE(b1, rotate_7); ROTATE(b2, rotate_7);

unsigned int attribute_hidden
__chacha20_power8_blocks4 (uint32_t *state, uint8_t *dst, const uint8_t *src,
			   size_t nblks)
{
  vector4x_u32 counters_0123 = { 0, 1, 2, 3 };
  vector4x_u32 counter_4 = { 4, 0, 0, 0 };
  vector4x_u32 rotate_16 = { 16, 16, 16, 16 };
  vector4x_u32 rotate_12 = { 12, 12, 12, 12 };
  vector4x_u32 rotate_8 = { 8, 8, 8, 8 };
  vector4x_u32 rotate_7 = { 7, 7, 7, 7 };
  vector4x_u32 state0, state1, state2, state3;
  vector4x_u32 v0, v1, v2, v3, v4, v5, v6, v7;
  vector4x_u32 v8, v9, v10, v11, v12, v13, v14, v15;
  vector4x_u32 tmp;
  int i;

  /* Force preload of constants to vector registers.  */
  __asm__ ("": "+v" (counters_0123) :: "memory");
  __asm__ ("": "+v" (counter_4) :: "memory");
  __asm__ ("": "+v" (rotate_16) :: "memory");
  __asm__ ("": "+v" (rotate_12) :: "memory");
  __asm__ ("": "+v" (rotate_8) :: "memory");
  __asm__ ("": "+v" (rotate_7) :: "memory");

  state0 = vec_vsx_ld (0 * 16, state);
  state1 = vec_vsx_ld (1 * 16, state);
  state2 = vec_vsx_ld (2 * 16, state);
  state3 = vec_vsx_ld (3 * 16, state);

  do
    {
      v0 = vec_splat (state0, 0);
      v1 = vec_splat (state0, 1);
      v2 = vec_splat (state0, 2);
      v3 = vec_splat (state0, 3);
      v4 = vec_splat (state1, 0);
      v5 = vec_splat (state1, 1);
      v6 = vec_splat (state1, 2);
      v7 = vec_splat (state1, 3);
      v8 = vec_splat (state2, 0);
      v9 = vec_splat (state2, 1);
      v10 = vec_splat (state2, 2);
      v11 = vec_splat (state2, 3);
      v12 = vec_splat (state3, 0);
      v13 = vec_splat (state3, 1);
      v14 = vec_splat (state3, 2);
      v15 = vec_splat (state3, 3);

      v12 += counters_0123;
      v13 -= vec_cmplt (v12, counters_0123);

      for (i = 20; i > 0; i -= 2)
	{
	  QUARTERROUND2 (v0, v4,  v8, v12,   v1, v5,  v9, v13)
	  QUARTERROUND2 (v2, v6, v10, v14,   v3, v7, v11, v15)
	  QUARTERROUND2 (v0, v5, v10, v15,   v1, v6, v11, v12)
	  QUARTERROUND2 (v2, v7,  v8, v13,   v3, v4,  v9, v14)
	}

      v0 += vec_splat (state0, 0);
      v1 += vec_splat (state0, 1);
      v2 += vec_splat (state0, 2);
      v3 += vec_splat (state0, 3);
      v4 += vec_splat (state1, 0);
      v5 += vec_splat (state1, 1);
      v6 += vec_splat (state1, 2);
      v7 += vec_splat (state1, 3);
      v8 += vec_splat (state2, 0);
      v9 += vec_splat (state2, 1);
      v10 += vec_splat (state2, 2);
      v11 += vec_splat (state2, 3);
      tmp = vec_splat( state3, 0);
      tmp += counters_0123;
      v12 += tmp;
      v13 += vec_splat (state3, 1) - vec_cmplt (tmp, counters_0123);
      v14 += vec_splat (state3, 2);
      v15 += vec_splat (state3, 3);
      ADD_U64 (state3, counter_4);

      transpose_4x4 (v0, v1, v2, v3);
      transpose_4x4 (v4, v5, v6, v7);
      transpose_4x4 (v8, v9, v10, v11);
      transpose_4x4 (v12, v13, v14, v15);

      vec_store_le (v0, (64 * 0 + 16 * 0), dst);
      vec_store_le (v1, (64 * 1 + 16 * 0), dst);
      vec_store_le (v2, (64 * 2 + 16 * 0), dst);
      vec_store_le (v3, (64 * 3 + 16 * 0), dst);

      vec_store_le (v4, (64 * 0 + 16 * 1), dst);
      vec_store_le (v5, (64 * 1 + 16 * 1), dst);
      vec_store_le (v6, (64 * 2 + 16 * 1), dst);
      vec_store_le (v7, (64 * 3 + 16 * 1), dst);

      vec_store_le (v8, (64 * 0 + 16 * 2), dst);
      vec_store_le (v9, (64 * 1 + 16 * 2), dst);
      vec_store_le (v10, (64 * 2 + 16 * 2), dst);
      vec_store_le (v11, (64 * 3 + 16 * 2), dst);

      vec_store_le (v12, (64 * 0 + 16 * 3), dst);
      vec_store_le (v13, (64 * 1 + 16 * 3), dst);
      vec_store_le (v14, (64 * 2 + 16 * 3), dst);
      vec_store_le (v15, (64 * 3 + 16 * 3), dst);

      src += 4*64;
      dst += 4*64;

      nblks -= 4;
    }
  while (nblks);

  vec_vsx_st (state3, 3 * 16, state);

  return 0;
}
