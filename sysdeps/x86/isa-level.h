/* Header defining the minimum x86 ISA level
   Copyright (C) 2022 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file.  (The Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _ISA_LEVEL_H
#define _ISA_LEVEL_H

#if defined __SSE__ && defined __SSE2__
/* NB: ISAs, excluding MMX, in x86-64 ISA level baseline are used.  */
# define __X86_ISA_V1 1
#else
# define __X86_ISA_V1 0
#endif

#if __X86_ISA_V1 && defined __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16               \
    && defined HAVE_X86_LAHF_SAHF && defined __POPCNT__ && defined __SSE3__   \
    && defined __SSSE3__ && defined __SSE4_1__ && defined __SSE4_2__
/* NB: ISAs in x86-64 ISA level v2 are used.  */
# define __X86_ISA_V2 1
#else
# define __X86_ISA_V2 0
#endif

#if __X86_ISA_V2 && defined __AVX__ && defined __AVX2__ && defined __F16C__   \
    && defined __FMA__ && defined __LZCNT__ && defined HAVE_X86_MOVBE         \
    && defined __BMI__ && defined __BMI2__
/* NB: ISAs in x86-64 ISA level v3 are used.  */
# define __X86_ISA_V3 1
#else
# define __X86_ISA_V3 0
#endif

#if __X86_ISA_V3 && defined __AVX512F__ && defined __AVX512BW__               \
    && defined __AVX512CD__ && defined __AVX512DQ__ && defined __AVX512VL__
/* NB: ISAs in x86-64 ISA level v4 are used.  */
# define __X86_ISA_V4 1
#else
# define __X86_ISA_V4 0
#endif

#define MINIMUM_X86_ISA_LEVEL                                                 \
  (__X86_ISA_V1 + __X86_ISA_V2 + __X86_ISA_V3 + __X86_ISA_V4)


/*
 * CPU Features that are hard coded as enabled depending on ISA build
 *   level.
 *    - Values > 0 features are always ENABLED if:
 *          Value >= MINIMUM_X86_ISA_LEVEL
 */


/* ISA level >= 4 guaranteed includes.  */
#define AVX512VL_X86_ISA_LEVEL 4
#define AVX512BW_X86_ISA_LEVEL 4

/* ISA level >= 3 guaranteed includes.  */
#define AVX2_X86_ISA_LEVEL 3
#define BMI2_X86_ISA_LEVEL 3

/*
 * NB: This may not be fully assumable for ISA level >= 3. From
 * looking over the architectures supported in cpu-features.h the
 * following CPUs may have an issue with this being default set:
 *      - AMD Excavator
 */
#define AVX_Fast_Unaligned_Load_X86_ISA_LEVEL 3

/*
 * KNL (the only cpu that sets this supported in cpu-features.h)
 * builds with ISA V1 so this shouldn't harm any architectures.
 */
#define Prefer_No_VZEROUPPER_X86_ISA_LEVEL 3

#define ISA_SHOULD_BUILD(isa_build_level)                              \
  (MINIMUM_X86_ISA_LEVEL <= (isa_build_level) && IS_IN (libc))         \
   || defined ISA_DEFAULT_IMPL

#endif
