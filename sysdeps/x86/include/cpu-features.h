/* Data structure for x86 CPU features.
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

#ifndef	_PRIVATE_CPU_FEATURES_H
#define	_PRIVATE_CPU_FEATURES_H	1

#ifdef _CPU_FEATURES_H
# error this should be impossible
#endif

#ifndef _ISOMAC
/* Get most of the contents from the public header, but we define a
   different `struct cpu_features' type for private use.  */
# define cpu_features		cpu_features_public
# define __x86_get_cpu_features	__x86_get_cpu_features_public
#endif

#include <sysdeps/x86/sys/platform/x86.h>

#ifndef _ISOMAC

# undef	cpu_features
# undef __x86_get_cpu_features
# define __get_cpu_features()	__x86_get_cpu_features (0)

enum
{
  /* The integer bit array index for the first set of preferred feature
     bits.  */
  PREFERRED_FEATURE_INDEX_1 = 0,
  /* The current maximum size of the feature integer bit array.  */
  PREFERRED_FEATURE_INDEX_MAX
};

/* Only used directly in cpu-features.c.  */
# define CPU_FEATURE_SET(ptr, name) \
  ptr->features[index_cpu_##name].usable.reg_##name |= bit_cpu_##name;
# define CPU_FEATURE_UNSET(ptr, name) \
  ptr->features[index_cpu_##name].usable.reg_##name &= ~bit_cpu_##name;
# define CPU_FEATURE_SET_USABLE(ptr, name) \
  ptr->features[index_cpu_##name].usable.reg_##name \
     |= ptr->features[index_cpu_##name].cpuid.reg_##name & bit_cpu_##name;
# define CPU_FEATURE_PREFERRED_P(ptr, name) \
  ((ptr->preferred[index_arch_##name] & bit_arch_##name) != 0)
# define CPU_FEATURE_CPU_P(ptr, name) \
  CPU_FEATURE_CHECK_P (ptr, name, cpuid)

/* HAS_CPU_FEATURE evaluates to true if CPU supports the feature.  */
# undef HAS_CPU_FEATURE
# define HAS_CPU_FEATURE(name) \
  CPU_FEATURE_CPU_P (__x86_get_cpu_features (0), name)
/* CPU_FEATURE_USABLE evaluates to true if the feature is usable.  */
# undef CPU_FEATURE_USABLE
# define CPU_FEATURE_USABLE(name) \
  CPU_FEATURE_USABLE_P (__x86_get_cpu_features (0), name)
/* CPU_FEATURE_PREFER evaluates to true if we prefer the feature at
   runtime.  */
# define CPU_FEATURE_PREFERRED(name) \
  CPU_FEATURE_PREFERRED_P(__get_cpu_features (), name)

# define CPU_FEATURES_CPU_P(ptr, name) \
  CPU_FEATURE_CPU_P (ptr, name)
# define CPU_FEATURES_ARCH_P(ptr, name) \
  CPU_FEATURE_PREFERRED_P (ptr, name)
# define HAS_ARCH_FEATURE(name) \
  CPU_FEATURE_PREFERRED (name)

/* PREFERRED_FEATURE_INDEX_1.  */
# define bit_arch_I586				(1u << 0)
# define bit_arch_I686				(1u << 1)
# define bit_arch_Fast_Rep_String		(1u << 2)
# define bit_arch_Fast_Copy_Backward		(1u << 3)
# define bit_arch_Fast_Unaligned_Load		(1u << 4)
# define bit_arch_Fast_Unaligned_Copy		(1u << 5)
# define bit_arch_Slow_BSF			(1u << 6)
# define bit_arch_Slow_SSE4_2			(1u << 7)
# define bit_arch_AVX_Fast_Unaligned_Load	(1u << 8)
# define bit_arch_Prefer_MAP_32BIT_EXEC		(1u << 9)
# define bit_arch_Prefer_PMINUB_for_stringop	(1u << 10)
# define bit_arch_Prefer_No_VZEROUPPER		(1u << 11)
# define bit_arch_Prefer_ERMS			(1u << 12)
# define bit_arch_Prefer_FSRM			(1u << 13)
# define bit_arch_Prefer_No_AVX512		(1u << 14)
# define bit_arch_MathVec_Prefer_No_AVX512	(1u << 15)

# define index_arch_Fast_Rep_String		PREFERRED_FEATURE_INDEX_1
# define index_arch_Fast_Copy_Backward		PREFERRED_FEATURE_INDEX_1
# define index_arch_Slow_BSF			PREFERRED_FEATURE_INDEX_1
# define index_arch_Fast_Unaligned_Load		PREFERRED_FEATURE_INDEX_1
# define index_arch_Prefer_PMINUB_for_stringop 	PREFERRED_FEATURE_INDEX_1
# define index_arch_Fast_Unaligned_Copy		PREFERRED_FEATURE_INDEX_1
# define index_arch_I586			PREFERRED_FEATURE_INDEX_1
# define index_arch_I686			PREFERRED_FEATURE_INDEX_1
# define index_arch_Slow_SSE4_2			PREFERRED_FEATURE_INDEX_1
# define index_arch_AVX_Fast_Unaligned_Load	PREFERRED_FEATURE_INDEX_1
# define index_arch_Prefer_MAP_32BIT_EXEC	PREFERRED_FEATURE_INDEX_1
# define index_arch_Prefer_No_VZEROUPPER	PREFERRED_FEATURE_INDEX_1
# define index_arch_Prefer_ERMS			PREFERRED_FEATURE_INDEX_1
# define index_arch_Prefer_No_AVX512		PREFERRED_FEATURE_INDEX_1
# define index_arch_MathVec_Prefer_No_AVX512	PREFERRED_FEATURE_INDEX_1
# define index_arch_Prefer_FSRM			PREFERRED_FEATURE_INDEX_1

/* XCR0 Feature flags.  */
# define bit_XMM_state		(1u << 1)
# define bit_YMM_state		(1u << 2)
# define bit_Opmask_state	(1u << 5)
# define bit_ZMM0_15_state	(1u << 6)
# define bit_ZMM16_31_state	(1u << 7)
# define bit_XTILECFG_state	(1u << 17)
# define bit_XTILEDATA_state	(1u << 18)

struct cpu_features
{
  struct cpu_features_basic basic;
  struct cpuid_features features[COMMON_CPUID_INDEX_MAX];
  unsigned int preferred[PREFERRED_FEATURE_INDEX_MAX];
  /* The state size for XSAVEC or XSAVE.  The type must be unsigned long
     int so that we use

	sub xsave_state_size_offset(%rip) %RSP_LP

     in _dl_runtime_resolve.  */
  unsigned long int xsave_state_size;
  /* The full state size for XSAVE when XSAVEC is disabled by

     GLIBC_TUNABLES=glibc.cpu.hwcaps=-XSAVEC
   */
  unsigned int xsave_state_full_size;
  /* Data cache size for use in memory and string routines, typically
     L1 size.  */
  unsigned long int data_cache_size;
  /* Shared cache size for use in memory and string routines, typically
     L2 or L3 size.  */
  unsigned long int shared_cache_size;
  /* Threshold to use non temporal store.  */
  unsigned long int non_temporal_threshold;
  /* Threshold to use "rep movsb".  */
  unsigned long int rep_movsb_threshold;
  /* Threshold to use "rep stosb".  */
  unsigned long int rep_stosb_threshold;
};

# if defined (_LIBC) && !IS_IN (nonlib)
/* Unused for x86.  */
#  define INIT_ARCH()
#  define __x86_get_cpu_features(max) (&GLRO(dl_x86_cpu_features))
extern void _dl_x86_init_cpu_features (void) attribute_hidden;
# endif

# ifdef __x86_64__
#  define HAS_CPUID 1
# elif (defined __i586__ || defined __pentium__	\
	|| defined __geode__ || defined __k6__)
#  define HAS_CPUID 1
#  define HAS_I586 1
#  define HAS_I686 HAS_ARCH_FEATURE (I686)
# elif defined __i486__
#  define HAS_CPUID 0
#  define HAS_I586 HAS_ARCH_FEATURE (I586)
#  define HAS_I686 HAS_ARCH_FEATURE (I686)
# else
#  define HAS_CPUID 1
#  define HAS_I586 1
#  define HAS_I686 1
# endif

#endif /* !_ISOMAC */

#endif /* include/cpu-features.h */
