/* Test CPU feature data against __builtin_cpu_supports.
   This file is part of the GNU C Library.
   Copyright (C) 2020 Free Software Foundation, Inc.

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

#include <sys/platform/x86.h>
#include <stdio.h>

int
check_supports (int supports, int usable, const char *supports_name,
		const char *name)
{
  printf ("Checking %s:\n", name);
  printf ("  %s: %d\n", name, usable);
  printf ("  __builtin_cpu_supports (%s): %d\n",
	  supports_name, supports);

  if ((supports != 0) != (usable != 0))
    {
      printf (" *** failure ***\n");
      return 1;
    }

  return 0;
}

#define CHECK_SUPPORTS(str, name) \
  check_supports (__builtin_cpu_supports (#str), \
		  CPU_FEATURE_USABLE (name), \
		  #str, "HAS_CPU_FEATURE (" #name ")");

static int
do_test (int argc, char **argv)
{
  int fails = 0;

#if __GNUC_PREREQ (11, 0)
  fails += CHECK_SUPPORTS (adx, ADX);
#endif
#if __GNUC_PREREQ (6, 0)
  fails += CHECK_SUPPORTS (aes, AES);
#endif
#if __GNUC_PREREQ (11, 1)
  fails += CHECK_SUPPORTS (amx_bf16, AMX_BF16);
  fails += CHECK_SUPPORTS (amx_int8, AMX_INT8);
  fails += CHECK_SUPPORTS (amx_tile, AMX_TILE);
#endif
  fails += CHECK_SUPPORTS (avx, AVX);
  fails += CHECK_SUPPORTS (avx2, AVX2);
#if __GNUC_PREREQ (7, 0)
  fails += CHECK_SUPPORTS (avx5124fmaps, AVX512_4FMAPS);
  fails += CHECK_SUPPORTS (avx5124vnniw, AVX512_4VNNIW);
#endif
#if __GNUC_PREREQ (10, 0)
  fails += CHECK_SUPPORTS (avx512bf16, AVX512_BF16);
#endif
#if __GNUC_PREREQ (8, 0)
  fails += CHECK_SUPPORTS (avx512bitalg, AVX512_BITALG);
#endif
#if __GNUC_PREREQ (6, 0)
  fails += CHECK_SUPPORTS (avx512ifma, AVX512_IFMA);
  fails += CHECK_SUPPORTS (avx512vbmi, AVX512_VBMI);
#endif
#if __GNUC_PREREQ (8, 0)
  fails += CHECK_SUPPORTS (avx512vbmi2, AVX512_VBMI2);
  fails += CHECK_SUPPORTS (avx512vnni, AVX512_VNNI);
#endif
#if __GNUC_PREREQ (10, 0)
  fails += CHECK_SUPPORTS (avx512vp2intersect, AVX512_VP2INTERSECT);
#endif
#if __GNUC_PREREQ (7, 0)
  fails += CHECK_SUPPORTS (avx512vpopcntdq, AVX512_VPOPCNTDQ);
#endif
#if __GNUC_PREREQ (6, 0)
  fails += CHECK_SUPPORTS (avx512bw, AVX512BW);
  fails += CHECK_SUPPORTS (avx512cd, AVX512CD);
  fails += CHECK_SUPPORTS (avx512er, AVX512ER);
  fails += CHECK_SUPPORTS (avx512dq, AVX512DQ);
#endif
#if __GNUC_PREREQ (5, 0)
  fails += CHECK_SUPPORTS (avx512f, AVX512F);
#endif
#if __GNUC_PREREQ (6, 0)
  fails += CHECK_SUPPORTS (avx512pf, AVX512PF);
  fails += CHECK_SUPPORTS (avx512vl, AVX512VL);
#endif
#if __GNUC_PREREQ (5, 0)
  fails += CHECK_SUPPORTS (bmi, BMI1);
  fails += CHECK_SUPPORTS (bmi2, BMI2);
#endif
#if __GNUC_PREREQ (11, 0)
  fails += CHECK_SUPPORTS (cldemote, CLDEMOTE);
  fails += CHECK_SUPPORTS (clflushopt, CLFLUSHOPT);
  fails += CHECK_SUPPORTS (clwb, CLWB);
#endif
  fails += CHECK_SUPPORTS (cmov, CMOV);
#if __GNUC_PREREQ (11, 0)
  fails += CHECK_SUPPORTS (cmpxchg16b, CMPXCHG16B);
  fails += CHECK_SUPPORTS (cmpxchg8b, CX8);
  fails += CHECK_SUPPORTS (enqcmd, ENQCMD);
  fails += CHECK_SUPPORTS (f16c, F16C);
#endif
#if __GNUC_PREREQ (4, 9)
  fails += CHECK_SUPPORTS (fma, FMA);
  fails += CHECK_SUPPORTS (fma4, FMA4);
#endif
#if __GNUC_PREREQ (11, 0)
  fails += CHECK_SUPPORTS (fsgsbase, FSGSBASE);
  fails += CHECK_SUPPORTS (fxsave, FXSR);
#endif
#if __GNUC_PREREQ (8, 0)
  fails += CHECK_SUPPORTS (gfni, GFNI);
#endif
#if __GNUC_PREREQ (11, 0)
  fails += CHECK_SUPPORTS (hle, HLE);
  fails += CHECK_SUPPORTS (ibt, IBT);
  fails += CHECK_SUPPORTS (lahf_lm, LAHF64_SAHF64);
  fails += CHECK_SUPPORTS (lm, LM);
  fails += CHECK_SUPPORTS (lwp, LWP);
  fails += CHECK_SUPPORTS (lzcnt, LZCNT);
#endif
  fails += CHECK_SUPPORTS (mmx, MMX);
#if __GNUC_PREREQ (11, 0)
  fails += CHECK_SUPPORTS (movbe, MOVBE);
  fails += CHECK_SUPPORTS (movdiri, MOVDIRI);
  fails += CHECK_SUPPORTS (movdir64b, MOVDIR64B);
  fails += CHECK_SUPPORTS (osxsave, OSXSAVE);
  fails += CHECK_SUPPORTS (pconfig, PCONFIG);
  fails += CHECK_SUPPORTS (pku, PKU);
#endif
  fails += CHECK_SUPPORTS (popcnt, POPCNT);
#if __GNUC_PREREQ (11, 0)
  fails += CHECK_SUPPORTS (prefetchwt1, PREFETCHWT1);
  fails += CHECK_SUPPORTS (rdpid, RDPID);
  fails += CHECK_SUPPORTS (rdrnd, RDRAND);
  fails += CHECK_SUPPORTS (rdseed, RDSEED);
  fails += CHECK_SUPPORTS (rtm, RTM);
  fails += CHECK_SUPPORTS (serialize, SERIALIZE);
  fails += CHECK_SUPPORTS (sha, SHA);
  fails += CHECK_SUPPORTS (shstk, SHSTK);
#endif
  fails += CHECK_SUPPORTS (sse, SSE);
  fails += CHECK_SUPPORTS (sse2, SSE2);
  fails += CHECK_SUPPORTS (sse3, SSE3);
  fails += CHECK_SUPPORTS (sse4.1, SSE4_1);
  fails += CHECK_SUPPORTS (sse4.2, SSE4_2);
#if __GNUC_PREREQ (4, 9)
  fails += CHECK_SUPPORTS (sse4a, SSE4A);
#endif
  fails += CHECK_SUPPORTS (ssse3, SSSE3);
#if __GNUC_PREREQ (11, 0)
  fails += CHECK_SUPPORTS (tbm, TBM);
  fails += CHECK_SUPPORTS (tsxldtrk, TSXLDTRK);
  fails += CHECK_SUPPORTS (vaes, VAES);
#endif
#if __GNUC_PREREQ (8, 0)
  fails += CHECK_SUPPORTS (vpclmulqdq, VPCLMULQDQ);
#endif
#if __GNUC_PREREQ (11, 0)
  fails += CHECK_SUPPORTS (waitpkg, WAITPKG);
  fails += CHECK_SUPPORTS (wbnoinvd, WBNOINVD);
#endif
#if __GNUC_PREREQ (4, 9)
  fails += CHECK_SUPPORTS (xop, XOP);
#endif
#if __GNUC_PREREQ (11, 0)
  fails += CHECK_SUPPORTS (xsave, XSAVE);
  fails += CHECK_SUPPORTS (xsavec, XSAVEC);
  fails += CHECK_SUPPORTS (xsaveopt, XSAVEOPT);
  fails += CHECK_SUPPORTS (xsaves, XSAVES);
#endif

  printf ("%d differences between __builtin_cpu_supports and glibc code.\n",
	  fails);

  return (fails != 0);
}

#include "../../../test-skeleton.c"
