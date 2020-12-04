/* glibc-hwcaps subdirectory test.  x86_64 version.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <support/check.h>
#include <sys/param.h>
#include <sys/platform/x86.h>

extern int marker2 (void);
extern int marker3 (void);
extern int marker4 (void);

/* Return the x86-64-vN level, 1 for the baseline.  */
static int
compute_level (void)
{
  const struct cpu_features *cpu_features
    = __x86_get_cpu_features (COMMON_CPUID_INDEX_MAX);

 if (!(CPU_FEATURE_USABLE_P (cpu_features, CMPXCHG16B)
       && CPU_FEATURE_USABLE_P (cpu_features, LAHF64_SAHF64)
       && CPU_FEATURE_USABLE_P (cpu_features, POPCNT)
       && CPU_FEATURE_USABLE_P (cpu_features, MMX)
       && CPU_FEATURE_USABLE_P (cpu_features, SSE)
       && CPU_FEATURE_USABLE_P (cpu_features, SSE2)
       && CPU_FEATURE_USABLE_P (cpu_features, SSE3)
       && CPU_FEATURE_USABLE_P (cpu_features, SSSE3)
       && CPU_FEATURE_USABLE_P (cpu_features, SSE4_1)
       && CPU_FEATURE_USABLE_P (cpu_features, SSE4_2)))
   return 1;
 if (!(CPU_FEATURE_USABLE_P (cpu_features, AVX)
       && CPU_FEATURE_USABLE_P (cpu_features, AVX2)
       && CPU_FEATURE_USABLE_P (cpu_features, BMI1)
       && CPU_FEATURE_USABLE_P (cpu_features, BMI2)
       && CPU_FEATURE_USABLE_P (cpu_features, F16C)
       && CPU_FEATURE_USABLE_P (cpu_features, FMA)
       && CPU_FEATURE_USABLE_P (cpu_features, LZCNT)
       && CPU_FEATURE_USABLE_P (cpu_features, MOVBE)
       && CPU_FEATURE_USABLE_P (cpu_features, OSXSAVE)))
   return 2;
 if (!(CPU_FEATURE_USABLE_P (cpu_features, AVX512F)
       && CPU_FEATURE_USABLE_P (cpu_features, AVX512BW)
       && CPU_FEATURE_USABLE_P (cpu_features, AVX512CD)
       && CPU_FEATURE_USABLE_P (cpu_features, AVX512DQ)
       && CPU_FEATURE_USABLE_P (cpu_features, AVX512VL)))
   return 3;
 return 4;
}

static int
do_test (void)
{
  int level = compute_level ();
  printf ("info: detected x86-64 micro-architecture level: %d\n", level);
  TEST_COMPARE (marker2 (), MIN (level, 2));
  TEST_COMPARE (marker3 (), MIN (level, 3));
  TEST_COMPARE (marker4 (), MIN (level, 4));
  return 0;
}

#include <support/test-driver.c>
