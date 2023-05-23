/* strstr optimized with 512-bit AVX-512 instructions
   Copyright (C) 2022-2023 Free Software Foundation, Inc.
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

#include <immintrin.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#define FULL_MMASK64 0xffffffffffffffff
#define ONE_64BIT 0x1ull
#define ZMM_SIZE_IN_BYTES 64
#define PAGESIZE 4096

#define cvtmask64_u64(...) (uint64_t) (__VA_ARGS__)
#define kshiftri_mask64(x, y) ((x) >> (y))
#define kand_mask64(x, y) ((x) & (y))

/*
 Returns the index of the first edge within the needle, returns 0 if no edge
 is found. Example: 'ab' is the first edge in 'aaaaaaaaaabaarddg'
 */
static inline size_t
find_edge_in_needle (const char *ned)
{
  size_t ind = 0;
  while (ned[ind + 1] != '\0')
    {
      if (ned[ind] != ned[ind + 1])
        return ind;
      else
        ind = ind + 1;
    }
  return 0;
}

/*
 Compare needle with haystack byte by byte at specified location
 */
static inline bool
verify_string_match (const char *hay, const size_t hay_index, const char *ned,
                     size_t ind)
{
  while (ned[ind] != '\0')
    {
      if (ned[ind] != hay[hay_index + ind])
        return false;
      ind = ind + 1;
    }
  return true;
}

/*
 Compare needle with haystack at specified location. The first 64 bytes are
 compared using a ZMM register.
 */
static inline bool
verify_string_match_avx512 (const char *hay, const size_t hay_index,
                            const char *ned, const __mmask64 ned_mask,
                            const __m512i ned_zmm)
{
  /* check first 64 bytes using zmm and then scalar */
  __m512i hay_zmm = _mm512_loadu_si512 (hay + hay_index); // safe to do so
  __mmask64 match = _mm512_mask_cmpneq_epi8_mask (ned_mask, hay_zmm, ned_zmm);
  if (match != 0x0) // failed the first few chars
    return false;
  else if (ned_mask == FULL_MMASK64)
    return verify_string_match (hay, hay_index, ned, ZMM_SIZE_IN_BYTES);
  return true;
}

char *
__strstr_avx512 (const char *haystack, const char *ned)
{
  char first = ned[0];
  if (first == '\0')
    return (char *)haystack;
  if (ned[1] == '\0')
    return (char *)strchr (haystack, ned[0]);

  size_t edge = find_edge_in_needle (ned);

  /* ensure haystack is as long as the pos of edge in needle */
  for (int ii = 0; ii < edge; ++ii)
    {
      if (haystack[ii] == '\0')
        return NULL;
    }

  /*
   Load 64 bytes of the needle and save it to a zmm register
   Read one cache line at a time to avoid loading across a page boundary
   */
  __mmask64 ned_load_mask = _bzhi_u64 (
      FULL_MMASK64, 64 - ((uintptr_t) (ned) & 63));
  __m512i ned_zmm = _mm512_maskz_loadu_epi8 (ned_load_mask, ned);
  __mmask64 ned_nullmask
      = _mm512_mask_testn_epi8_mask (ned_load_mask, ned_zmm, ned_zmm);

  if (__glibc_unlikely (ned_nullmask == 0x0))
    {
      ned_zmm = _mm512_loadu_si512 (ned);
      ned_nullmask = _mm512_testn_epi8_mask (ned_zmm, ned_zmm);
      ned_load_mask = ned_nullmask ^ (ned_nullmask - ONE_64BIT);
      if (ned_nullmask != 0x0)
        ned_load_mask = ned_load_mask >> 1;
    }
  else
    {
      ned_load_mask = ned_nullmask ^ (ned_nullmask - ONE_64BIT);
      ned_load_mask = ned_load_mask >> 1;
    }
  const __m512i ned0 = _mm512_set1_epi8 (ned[edge]);
  const __m512i ned1 = _mm512_set1_epi8 (ned[edge + 1]);

  /*
   Read the bytes of haystack in the current cache line
   */
  size_t hay_index = edge;
  __mmask64 loadmask = _bzhi_u64 (
      FULL_MMASK64, 64 - ((uintptr_t) (haystack + hay_index) & 63));
  /* First load is a partial cache line */
  __m512i hay0 = _mm512_maskz_loadu_epi8 (loadmask, haystack + hay_index);
  /* Search for NULL and compare only till null char */
  uint64_t nullmask
      = cvtmask64_u64 (_mm512_mask_testn_epi8_mask (loadmask, hay0, hay0));
  uint64_t cmpmask = nullmask ^ (nullmask - ONE_64BIT);
  cmpmask = cmpmask & cvtmask64_u64 (loadmask);
  /* Search for the 2 characters of needle */
  __mmask64 k0 = _mm512_cmpeq_epi8_mask (hay0, ned0);
  __mmask64 k1 = _mm512_cmpeq_epi8_mask (hay0, ned1);
  k1 = kshiftri_mask64 (k1, 1);
  /* k2 masks tell us if both chars from needle match */
  uint64_t k2 = cvtmask64_u64 (kand_mask64 (k0, k1)) & cmpmask;
  /* For every match, search for the entire needle for a full match */
  while (k2)
    {
      uint64_t bitcount = _tzcnt_u64 (k2);
      k2 = _blsr_u64 (k2);
      size_t match_pos = hay_index + bitcount - edge;
      if (((uintptr_t) (haystack + match_pos) & (PAGESIZE - 1))
          < PAGESIZE - 1 - ZMM_SIZE_IN_BYTES)
        {
          /*
           * Use vector compare as long as you are not crossing a page
           */
          if (verify_string_match_avx512 (haystack, match_pos, ned,
                                          ned_load_mask, ned_zmm))
            return (char *)haystack + match_pos;
        }
      else
        {
          if (verify_string_match (haystack, match_pos, ned, 0))
            return (char *)haystack + match_pos;
        }
    }
  /* We haven't checked for potential match at the last char yet */
  haystack = (const char *)(((uintptr_t) (haystack + hay_index) | 63));
  hay_index = 0;

  /*
   Loop over one cache line at a time to prevent reading over page
   boundary
   */
  __m512i hay1;
  while (nullmask == 0)
    {
      hay0 = _mm512_loadu_si512 (haystack + hay_index);
      hay1 = _mm512_load_si512 (haystack + hay_index
                                + 1); // Always 64 byte aligned
      nullmask = cvtmask64_u64 (_mm512_testn_epi8_mask (hay1, hay1));
      /* Compare only till null char */
      cmpmask = nullmask ^ (nullmask - ONE_64BIT);
      k0 = _mm512_cmpeq_epi8_mask (hay0, ned0);
      k1 = _mm512_cmpeq_epi8_mask (hay1, ned1);
      /* k2 masks tell us if both chars from needle match */
      k2 = cvtmask64_u64 (kand_mask64 (k0, k1)) & cmpmask;
      /* For every match, compare full strings for potential match */
      while (k2)
        {
          uint64_t bitcount = _tzcnt_u64 (k2);
          k2 = _blsr_u64 (k2);
          size_t match_pos = hay_index + bitcount - edge;
          if (((uintptr_t) (haystack + match_pos) & (PAGESIZE - 1))
              < PAGESIZE - 1 - ZMM_SIZE_IN_BYTES)
            {
              /*
               * Use vector compare as long as you are not crossing a page
               */
              if (verify_string_match_avx512 (haystack, match_pos, ned,
                                              ned_load_mask, ned_zmm))
                return (char *)haystack + match_pos;
            }
          else
            {
              /* Compare byte by byte */
              if (verify_string_match (haystack, match_pos, ned, 0))
                return (char *)haystack + match_pos;
            }
        }
      hay_index += ZMM_SIZE_IN_BYTES;
    }
  return NULL;
}
