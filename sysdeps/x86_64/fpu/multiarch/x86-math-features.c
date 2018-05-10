/* Initialize CPU features for use by the math library.
   This file is part of the GNU C Library.
   Copyright (C) 2008-2018 Free Software Foundation, Inc.

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
   <http://www.gnu.org/licenses/>.  */

#include <x86-math-features.h>
#include <atomic.h>
#include <cpuid.h>
#include <cpu-features.h>
#include <stdbool.h>

static unsigned int features;

unsigned int
__x86_math_features (void)
{
  unsigned int features_local = atomic_load_relaxed (&features);
  if (features_local != 0)
    /* At least the initialization bit is set, which means that we
       have a proper value.  */
    return features_local;

  /* Perform initialization.  */
  features_local = x86_math_feature_initialized;

  unsigned int eax, ebx, ecx, edx;
  unsigned int max_cpuid;
  __cpuid (0, max_cpuid, ebx, ecx, edx);
  bool cpu_amd = ebx == 0x68747541 && ecx == 0x444d4163 && edx == 0x69746e65;

  if (max_cpuid >= 7)
    {
      __cpuid (1, eax, ebx, ecx, edx);
      bool flag_fma = ecx & bit_cpu_FMA;
      bool flag_osxsave = ecx & bit_cpu_OSXSAVE;
      bool flag_avx = ecx & bit_cpu_AVX;
      bool flag_sse41 = ecx & bit_cpu_SSE4_1;

      if (flag_sse41)
        features_local |= x86_math_feature_sse41;

      __cpuid_count (7, 0, eax, ebx, ecx, edx);
      bool flag_avx2 = ebx & bit_cpu_AVX2;

      if (flag_osxsave)
        {
          unsigned int xcrlow;
          unsigned int xcrhigh;
          asm ("xgetbv" : "=a" (xcrlow), "=d" (xcrhigh) : "c" (0));
          bool ymm_xmm_usable
            = (xcrlow & (bit_YMM_state | bit_XMM_state))
              == (bit_YMM_state | bit_XMM_state);

          /* Is YMM and XMM state usable?  */
          if (ymm_xmm_usable)
            {
              if (flag_avx)
                {
                  features_local |= x86_math_feature_avx;
                  if (flag_avx2)
                    features_local |= x86_math_feature_avx2;
                  if (flag_fma)
                    features_local |= x86_math_feature_fma;

                  if (cpu_amd)
                    {
                      __cpuid (0x80000000, eax, ebx, ecx, edx);
                      if (eax >= 0x80000001)
                        {
                          __cpuid (0x80000001, eax, ebx, ecx, edx);
                          bool flag_fma4 = ecx & bit_cpu_FMA4;
                          if (flag_fma4)
                            features_local |= x86_math_feature_fma4;
                        }
                    }
                }
            }
        }
    }

  atomic_store_relaxed (&features, features_local);
  return features_local;
}
