/* Multiple versions of bzero.
   All versions must be listed in ifunc-impl-list.c.
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

/* Define multiple versions only for the definition in libc.  */
#if IS_IN (libc)
# define __bzero __redirect___bzero
# include <string.h>
# undef __bzero

# define SYMBOL_NAME __bzero
# include <init-arch.h>

extern __typeof (REDIRECT_NAME) OPTIMIZE1 (sse2_unaligned)
  attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE1 (sse2_unaligned_erms)
  attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE1 (avx2_unaligned) attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE1 (avx2_unaligned_erms)
  attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE1 (avx2_unaligned_rtm)
  attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE1 (avx2_unaligned_erms_rtm)
  attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE1 (evex_unaligned)
  attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE1 (evex_unaligned_erms)
  attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE1 (avx512_unaligned)
  attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE1 (avx512_unaligned_erms)
  attribute_hidden;

static inline void *
IFUNC_SELECTOR (void)
{
  const struct cpu_features* cpu_features = __get_cpu_features ();

  if (CPU_FEATURE_USABLE_P (cpu_features, AVX512F)
      && !CPU_FEATURES_ARCH_P (cpu_features, Prefer_No_AVX512))
    {
      if (CPU_FEATURE_USABLE_P (cpu_features, AVX512VL)
          && CPU_FEATURE_USABLE_P (cpu_features, AVX512BW)
          && CPU_FEATURE_USABLE_P (cpu_features, BMI2))
	{
	  if (CPU_FEATURE_USABLE_P (cpu_features, ERMS))
	    return OPTIMIZE1 (avx512_unaligned_erms);

	  return OPTIMIZE1 (avx512_unaligned);
	}
    }

  if (CPU_FEATURE_USABLE_P (cpu_features, AVX2))
    {
      if (CPU_FEATURE_USABLE_P (cpu_features, AVX512VL)
          && CPU_FEATURE_USABLE_P (cpu_features, AVX512BW)
          && CPU_FEATURE_USABLE_P (cpu_features, BMI2))
	{
	  if (CPU_FEATURE_USABLE_P (cpu_features, ERMS))
	    return OPTIMIZE1 (evex_unaligned_erms);

	  return OPTIMIZE1 (evex_unaligned);
	}

      if (CPU_FEATURE_USABLE_P (cpu_features, RTM))
	{
	  if (CPU_FEATURE_USABLE_P (cpu_features, ERMS))
	    return OPTIMIZE1 (avx2_unaligned_erms_rtm);

	  return OPTIMIZE1 (avx2_unaligned_rtm);
	}

      if (!CPU_FEATURES_ARCH_P (cpu_features, Prefer_No_VZEROUPPER))
	{
	  if (CPU_FEATURE_USABLE_P (cpu_features, ERMS))
	    return OPTIMIZE1 (avx2_unaligned_erms);

	  return OPTIMIZE1 (avx2_unaligned);
	}
    }

  if (CPU_FEATURE_USABLE_P (cpu_features, ERMS))
    return OPTIMIZE1 (sse2_unaligned_erms);

  return OPTIMIZE1 (sse2_unaligned);
}

libc_ifunc_redirected (__redirect___bzero, __bzero, IFUNC_SELECTOR ());

weak_alias (__bzero, bzero)
#endif
