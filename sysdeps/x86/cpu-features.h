/* This file is part of the GNU C Library.
   Copyright (C) 2008-2020 Free Software Foundation, Inc.

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

#ifndef cpu_features_h
#define cpu_features_h

enum
{
  /* The integer bit array index for the first set of internal feature
     bits.  */
  FEATURE_INDEX_1 = 0,
  FEATURE_INDEX_2,
  /* The current maximum size of the feature integer bit array.  */
  FEATURE_INDEX_MAX
};

enum
{
  COMMON_CPUID_INDEX_1 = 0,
  COMMON_CPUID_INDEX_7,
  COMMON_CPUID_INDEX_80000001,
  COMMON_CPUID_INDEX_D_ECX_1,
  COMMON_CPUID_INDEX_80000007,
  COMMON_CPUID_INDEX_80000008,
  /* Keep the following line at the end.  */
  COMMON_CPUID_INDEX_MAX
};

struct cpuid_registers
{
  unsigned int eax;
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;
};

enum cpu_features_kind
{
  arch_kind_unknown = 0,
  arch_kind_intel,
  arch_kind_amd,
  arch_kind_other
};

struct cpu_features_basic
{
  enum cpu_features_kind kind;
  int max_cpuid;
  unsigned int family;
  unsigned int model;
  unsigned int stepping;
};

struct cpu_features
{
  struct cpuid_registers cpuid[COMMON_CPUID_INDEX_MAX];
  unsigned int feature[FEATURE_INDEX_MAX];
  struct cpu_features_basic basic;
  /* The state size for XSAVEC or XSAVE.  The type must be unsigned long
     int so that we use

	sub xsave_state_size_offset(%rip) %RSP_LP

     in _dl_runtime_resolve.  */
  unsigned long int xsave_state_size;
  /* The full state size for XSAVE when XSAVEC is disabled by

     GLIBC_TUNABLES=glibc.cpu.hwcaps=-XSAVEC_Usable
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
};

/* Used from outside of glibc to get access to the CPU features
   structure.  */
extern const struct cpu_features *__get_cpu_features (void)
     __attribute__ ((const));

/* Only used directly in cpu-features.c.  */
# define CPU_FEATURES_CPU_P(ptr, name) \
  ((ptr->cpuid[index_cpu_##name].reg_##name & (bit_cpu_##name)) != 0)
# define CPU_FEATURES_ARCH_P(ptr, name) \
  ((ptr->feature[index_arch_##name] & (bit_arch_##name)) != 0)

/* HAS_CPU_FEATURE evaluates to true if CPU supports the feature.  */
#define HAS_CPU_FEATURE(name) \
  CPU_FEATURES_CPU_P (__get_cpu_features (), name)
/* HAS_ARCH_FEATURE evaluates to true if we may use the feature at
   runtime.  */
# define HAS_ARCH_FEATURE(name) \
  CPU_FEATURES_ARCH_P (__get_cpu_features (), name)
/* CPU_FEATURE_USABLE evaluates to true if the feature is usable.  */
#define CPU_FEATURE_USABLE(name)				\
  ((need_arch_feature_##name && HAS_ARCH_FEATURE (name##_Usable))	\
   || (!need_arch_feature_##name && HAS_CPU_FEATURE(name)))

/* Architecture features.  */

/* FEATURE_INDEX_1.  */
#define bit_arch_AVX_Usable			(1u << 0)
#define bit_arch_AVX2_Usable			(1u << 1)
#define bit_arch_AVX512F_Usable			(1u << 2)
#define bit_arch_AVX512CD_Usable		(1u << 3)
#define bit_arch_AVX512ER_Usable		(1u << 4)
#define bit_arch_AVX512PF_Usable		(1u << 5)
#define bit_arch_AVX512VL_Usable		(1u << 6)
#define bit_arch_AVX512DQ_Usable		(1u << 7)
#define bit_arch_AVX512BW_Usable		(1u << 8)
#define bit_arch_AVX512_4FMAPS_Usable		(1u << 9)
#define bit_arch_AVX512_4VNNIW_Usable		(1u << 10)
#define bit_arch_AVX512_BITALG_Usable		(1u << 11)
#define bit_arch_AVX512_IFMA_Usable		(1u << 12)
#define bit_arch_AVX512_VBMI_Usable		(1u << 13)
#define bit_arch_AVX512_VBMI2_Usable		(1u << 14)
#define bit_arch_AVX512_VNNI_Usable		(1u << 15)
#define bit_arch_AVX512_VPOPCNTDQ_Usable	(1u << 16)
#define bit_arch_FMA_Usable			(1u << 17)
#define bit_arch_FMA4_Usable			(1u << 18)
#define bit_arch_VAES_Usable			(1u << 19)
#define bit_arch_VPCLMULQDQ_Usable		(1u << 20)
#define bit_arch_XOP_Usable			(1u << 21)
#define bit_arch_XSAVEC_Usable			(1u << 22)

#define index_arch_AVX_Usable			FEATURE_INDEX_1
#define index_arch_AVX2_Usable			FEATURE_INDEX_1
#define index_arch_AVX512F_Usable		FEATURE_INDEX_1
#define index_arch_AVX512CD_Usable		FEATURE_INDEX_1
#define index_arch_AVX512ER_Usable		FEATURE_INDEX_1
#define index_arch_AVX512PF_Usable		FEATURE_INDEX_1
#define index_arch_AVX512VL_Usable		FEATURE_INDEX_1
#define index_arch_AVX512BW_Usable		FEATURE_INDEX_1
#define index_arch_AVX512DQ_Usable		FEATURE_INDEX_1
#define index_arch_AVX512_4FMAPS_Usable		FEATURE_INDEX_1
#define index_arch_AVX512_4VNNIW_Usable		FEATURE_INDEX_1
#define index_arch_AVX512_BITALG_Usable		FEATURE_INDEX_1
#define index_arch_AVX512_IFMA_Usable		FEATURE_INDEX_1
#define index_arch_AVX512_VBMI_Usable		FEATURE_INDEX_1
#define index_arch_AVX512_VBMI2_Usable		FEATURE_INDEX_1
#define index_arch_AVX512_VNNI_Usable		FEATURE_INDEX_1
#define index_arch_AVX512_VPOPCNTDQ_Usable	FEATURE_INDEX_1
#define index_arch_FMA_Usable			FEATURE_INDEX_1
#define index_arch_FMA4_Usable			FEATURE_INDEX_1
#define index_arch_VAES_Usable			FEATURE_INDEX_1
#define index_arch_VPCLMULQDQ_Usable		FEATURE_INDEX_1
#define index_arch_XOP_Usable			FEATURE_INDEX_1
#define index_arch_XSAVEC_Usable		FEATURE_INDEX_1

/* Unused.  Compiler will optimize them out.  */
#define bit_arch_SSE3_Usable			(1u << 0)
#define bit_arch_PCLMULQDQ_Usable		(1u << 0)
#define bit_arch_SSSE3_Usable			(1u << 0)
#define bit_arch_CMPXCHG16B_Usable		(1u << 0)
#define bit_arch_SSE4_1_Usable			(1u << 0)
#define bit_arch_SSE4_2_Usable			(1u << 0)
#define bit_arch_MOVBE_Usable			(1u << 0)
#define bit_arch_POPCNT_Usable			(1u << 0)
#define bit_arch_AES_Usable			(1u << 0)
#define bit_arch_XSAVE_Usable			(1u << 0)
#define bit_arch_OSXSAVE_Usable			(1u << 0)
#define bit_arch_F16C_Usable			(1u << 0)
#define bit_arch_RDRAND_Usable			(1u << 0)
#define bit_arch_FPU_Usable			(1u << 0)
#define bit_arch_TSC_Usable			(1u << 0)
#define bit_arch_MSR_Usable			(1u << 0)
#define bit_arch_CX8_Usable			(1u << 0)
#define bit_arch_SEP_Usable			(1u << 0)
#define bit_arch_CMOV_Usable			(1u << 0)
#define bit_arch_CLFSH_Usable			(1u << 0)
#define bit_arch_MMX_Usable			(1u << 0)
#define bit_arch_FXSR_Usable			(1u << 0)
#define bit_arch_SSE_Usable			(1u << 0)
#define bit_arch_SSE2_Usable			(1u << 0)
#define bit_arch_FSGSBASE_Usable		(1u << 0)
#define bit_arch_BMI1_Usable			(1u << 0)
#define bit_arch_HLE_Usable			(1u << 0)
#define bit_arch_BMI2_Usable			(1u << 0)
#define bit_arch_ERMS_Usable			(1u << 0)
#define bit_arch_RTM_Usable			(1u << 0)
#define bit_arch_RDSEED_Usable			(1u << 0)
#define bit_arch_ADX_Usable			(1u << 0)
#define bit_arch_CLFLUSHOPT_Usable		(1u << 0)
#define bit_arch_CLWB_Usable			(1u << 0)
#define bit_arch_SHA_Usable			(1u << 0)
#define bit_arch_PREFETCHWT1_Usable		(1u << 0)
#define bit_arch_GFNI_Usable			(1u << 0)
#define bit_arch_RDPID_Usable			(1u << 0)
#define bit_arch_CLDEMOTE_Usable		(1u << 0)
#define bit_arch_MOVDIRI_Usable			(1u << 0)
#define bit_arch_MOVDIR64B_Usable		(1u << 0)
#define bit_arch_FSRM_Usable			(1u << 0)
#define bit_arch_LAHF64_SAHF64_Usable		(1u << 0)
#define bit_arch_SVM_Usable			(1u << 0)
#define bit_arch_LZCNT_Usable			(1u << 0)
#define bit_arch_SSE4A_Usable			(1u << 0)
#define bit_arch_PREFETCHW_Usable		(1u << 0)
#define bit_arch_TBM_Usable			(1u << 0)
#define bit_arch_SYSCALL_SYSRET_Usable		(1u << 0)
#define bit_arch_RDTSCP_Usable			(1u << 0)
#define bit_arch_XSAVEOPT_Usable		(1u << 0)
#define bit_arch_XGETBV_ECX_1_Usable		(1u << 0)
#define bit_arch_XSAVES_Usable			(1u << 0)
#define bit_arch_INVARIANT_TSC_Usable		(1u << 0)
#define bit_arch_WBNOINVD_Usable		(1u << 0)

/* Unused.  Compiler will optimize them out.  */
#define index_arch_SSE3_Usable			FEATURE_INDEX_1
#define index_arch_PCLMULQDQ_Usable		FEATURE_INDEX_1
#define index_arch_SSSE3_Usable			FEATURE_INDEX_1
#define index_arch_CMPXCHG16B_Usable		FEATURE_INDEX_1
#define index_arch_SSE4_1_Usable		FEATURE_INDEX_1
#define index_arch_SSE4_2_Usable		FEATURE_INDEX_1
#define index_arch_MOVBE_Usable			FEATURE_INDEX_1
#define index_arch_POPCNT_Usable		FEATURE_INDEX_1
#define index_arch_AES_Usable			FEATURE_INDEX_1
#define index_arch_XSAVE_Usable			FEATURE_INDEX_1
#define index_arch_OSXSAVE_Usable		FEATURE_INDEX_1
#define index_arch_F16C_Usable			FEATURE_INDEX_1
#define index_arch_RDRAND_Usable		FEATURE_INDEX_1
#define index_arch_FPU_Usable			FEATURE_INDEX_1
#define index_arch_TSC_Usable			FEATURE_INDEX_1
#define index_arch_MSR_Usable			FEATURE_INDEX_1
#define index_arch_CX8_Usable			FEATURE_INDEX_1
#define index_arch_SEP_Usable			FEATURE_INDEX_1
#define index_arch_CMOV_Usable			FEATURE_INDEX_1
#define index_arch_CLFSH_Usable			FEATURE_INDEX_1
#define index_arch_MMX_Usable			FEATURE_INDEX_1
#define index_arch_FXSR_Usable			FEATURE_INDEX_1
#define index_arch_SSE_Usable			FEATURE_INDEX_1
#define index_arch_SSE2_Usable			FEATURE_INDEX_1
#define index_arch_FSGSBASE_Usable		FEATURE_INDEX_1
#define index_arch_BMI1_Usable			FEATURE_INDEX_1
#define index_arch_HLE_Usable			FEATURE_INDEX_1
#define index_arch_BMI2_Usable			FEATURE_INDEX_1
#define index_arch_ERMS_Usable			FEATURE_INDEX_1
#define index_arch_RTM_Usable			FEATURE_INDEX_1
#define index_arch_RDSEED_Usable		FEATURE_INDEX_1
#define index_arch_ADX_Usable			FEATURE_INDEX_1
#define index_arch_CLFLUSHOPT_Usable		FEATURE_INDEX_1
#define index_arch_CLWB_Usable			FEATURE_INDEX_1
#define index_arch_SHA_Usable			FEATURE_INDEX_1
#define index_arch_PREFETCHWT1_Usable		FEATURE_INDEX_1
#define index_arch_GFNI_Usable			FEATURE_INDEX_1
#define index_arch_RDPID_Usable			FEATURE_INDEX_1
#define index_arch_CLDEMOTE_Usable		FEATURE_INDEX_1
#define index_arch_MOVDIRI_Usable		FEATURE_INDEX_1
#define index_arch_MOVDIR64B_Usable		FEATURE_INDEX_1
#define index_arch_FSRM_Usable			FEATURE_INDEX_1
#define index_arch_LAHF64_SAHF64_Usable		FEATURE_INDEX_1
#define index_arch_LZCNT_Usable			FEATURE_INDEX_1
#define index_arch_SSE4A_Usable			FEATURE_INDEX_1
#define index_arch_PREFETCHW_Usable		FEATURE_INDEX_1
#define index_arch_TBM_Usable			FEATURE_INDEX_1
#define index_arch_SYSCALL_SYSRET_Usable	FEATURE_INDEX_1
#define index_arch_RDTSCP_Usable		FEATURE_INDEX_1
#define index_arch_XSAVEOPT_Usable		FEATURE_INDEX_1
#define index_arch_XGETBV_ECX_1_Usable		FEATURE_INDEX_1
#define index_arch_XSAVES_Usable		FEATURE_INDEX_1
#define index_arch_INVARIANT_TSC_Usable		FEATURE_INDEX_1
#define index_arch_WBNOINVD_Usable		FEATURE_INDEX_1

/* COMMON_CPUID_INDEX_1.  */

/* ECX.  */
#define	need_arch_feature_SSE3			0
#define	need_arch_feature_PCLMULQDQ		0
#define need_arch_feature_SSSE3			0
#define need_arch_feature_FMA			1
#define need_arch_feature_CMPXCHG16B		0
#define need_arch_feature_SSE4_1		0
#define need_arch_feature_SSE4_2		0
#define need_arch_feature_MOVBE			0
#define need_arch_feature_POPCNT		0
#define need_arch_feature_AES			0
#define need_arch_feature_XSAVE			0
#define need_arch_feature_OSXSAVE		0
#define need_arch_feature_AVX			1
#define need_arch_feature_F16C			0
#define need_arch_feature_RDRAND		0

/* EDX.  */
#define need_arch_feature_FPU			0
#define need_arch_feature_TSC			0
#define need_arch_feature_MSR			0
#define need_arch_feature_CX8			0
#define need_arch_feature_SEP			0
#define need_arch_feature_CMOV			0
#define need_arch_feature_CLFSH			0
#define need_arch_feature_MMX			0
#define need_arch_feature_FXSR			0
#define need_arch_feature_SSE			0
#define need_arch_feature_SSE2			0

/* COMMON_CPUID_INDEX_7.  */

/* EBX.  */
#define need_arch_feature_FSGSBASE		0
#define need_arch_feature_BMI1			0
#define need_arch_feature_HLE			0
#define need_arch_feature_AVX2			1
#define need_arch_feature_BMI2			0
#define need_arch_feature_ERMS			0
#define need_arch_feature_RTM			0
#define need_arch_feature_AVX512F		1
#define need_arch_feature_AVX512DQ		1
#define need_arch_feature_RDSEED		0
#define need_arch_feature_ADX			0
#define need_arch_feature_AVX512_IFMA		1
#define need_arch_feature_CLFLUSHOPT		0
#define need_arch_feature_CLWB			0
#define need_arch_feature_AVX512PF		1
#define need_arch_feature_AVX512ER		1
#define need_arch_feature_AVX512CD		1
#define need_arch_feature_SHA			0
#define need_arch_feature_AVX512BW		1
#define need_arch_feature_AVX512VL		1

/* ECX.  */
#define need_arch_feature_PREFETCHWT1		0
#define need_arch_feature_AVX512_VBMI		1
#define need_arch_feature_AVX512_VBMI2		1
#define need_arch_feature_GFNI			0
#define need_arch_feature_VAES			1
#define need_arch_feature_VPCLMULQDQ		1
#define need_arch_feature_AVX512_VNNI		1
#define need_arch_feature_AVX512_BITALG		1
#define need_arch_feature_AVX512_VPOPCNTDQ	1
#define need_arch_feature_RDPID			0
#define need_arch_feature_CLDEMOTE		0
#define need_arch_feature_MOVDIRI		0
#define need_arch_feature_MOVDIR64B		0

/* EDX.  */
#define need_arch_feature_AVX512_4VNNIW		1
#define need_arch_feature_AVX512_4FMAPS		1
#define need_arch_feature_FSRM			0

/* COMMON_CPUID_INDEX_80000001.  */

/* ECX.  */
#define need_arch_feature_LAHF64_SAHF64		0
#define need_arch_feature_LZCNT			0
#define need_arch_feature_SSE4A			0
#define need_arch_feature_PREFETCHW		0
#define need_arch_feature_XOP			1
#define need_arch_feature_FMA4			1
#define need_arch_feature_TBM			0
#define need_arch_feature_SYSCALL_SYSRET	0
#define need_arch_feature_RDTSCP		0
#define need_arch_feature_XSAVEOPT		0
#define need_arch_feature_XSAVEC		1
#define need_arch_feature_XGETBV_ECX_1		0
#define need_arch_feature_XSAVES		0
#define need_arch_feature_INVARIANT_TSC		0
#define need_arch_feature_WBNOINVD		0

/* CPU features.  */

/* COMMON_CPUID_INDEX_1.  */

/* ECX.  */
#define bit_cpu_SSE3		(1u << 0)
#define bit_cpu_PCLMULQDQ	(1u << 1)
#define bit_cpu_DTES64		(1u << 2)
#define bit_cpu_MONITOR		(1u << 3)
#define bit_cpu_DS_CPL		(1u << 4)
#define bit_cpu_VMX		(1u << 5)
#define bit_cpu_SMX		(1u << 6)
#define bit_cpu_EST		(1u << 7)
#define bit_cpu_TM2		(1u << 8)
#define bit_cpu_SSSE3		(1u << 9)
#define bit_cpu_CNXT_ID		(1u << 10)
#define bit_cpu_SDBG		(1u << 11)
#define bit_cpu_FMA		(1u << 12)
#define bit_cpu_CMPXCHG16B	(1u << 13)
#define bit_cpu_XTPRUPDCTRL	(1u << 14)
#define bit_cpu_PDCM		(1u << 15)
#define bit_cpu_PCID		(1u << 17)
#define bit_cpu_DCA		(1u << 18)
#define bit_cpu_SSE4_1		(1u << 19)
#define bit_cpu_SSE4_2		(1u << 20)
#define bit_cpu_X2APIC		(1u << 21)
#define bit_cpu_MOVBE		(1u << 22)
#define bit_cpu_POPCNT		(1u << 23)
#define bit_cpu_TSC_DEADLINE	(1u << 24)
#define bit_cpu_AES		(1u << 25)
#define bit_cpu_XSAVE		(1u << 26)
#define bit_cpu_OSXSAVE		(1u << 27)
#define bit_cpu_AVX		(1u << 28)
#define bit_cpu_F16C		(1u << 29)
#define bit_cpu_RDRAND		(1u << 30)

/* EDX.  */
#define bit_cpu_FPU		(1u << 0)
#define bit_cpu_VME		(1u << 1)
#define bit_cpu_DE		(1u << 2)
#define bit_cpu_PSE		(1u << 3)
#define bit_cpu_TSC		(1u << 4)
#define bit_cpu_MSR		(1u << 5)
#define bit_cpu_PAE		(1u << 6)
#define bit_cpu_MCE		(1u << 7)
#define bit_cpu_CX8		(1u << 8)
#define bit_cpu_APIC		(1u << 9)
#define bit_cpu_SEP		(1u << 11)
#define bit_cpu_MTRR		(1u << 12)
#define bit_cpu_PGE		(1u << 13)
#define bit_cpu_MCA		(1u << 14)
#define bit_cpu_CMOV		(1u << 15)
#define bit_cpu_PAT		(1u << 16)
#define bit_cpu_PSE_36		(1u << 17)
#define bit_cpu_PSN		(1u << 18)
#define bit_cpu_CLFSH		(1u << 20)
#define bit_cpu_DS		(1u << 21)
#define bit_cpu_ACPI		(1u << 22)
#define bit_cpu_MMX		(1u << 23)
#define bit_cpu_FXSR		(1u << 24)
#define bit_cpu_SSE		(1u << 25)
#define bit_cpu_SSE2		(1u << 26)
#define bit_cpu_SS		(1u << 27)
#define bit_cpu_HTT		(1u << 28)
#define bit_cpu_TM		(1u << 29)
#define bit_cpu_PBE		(1u << 31)

/* COMMON_CPUID_INDEX_7.  */

/* EBX.  */
#define bit_cpu_FSGSBASE	(1u << 0)
#define bit_cpu_TSC_ADJUST	(1u << 1)
#define bit_cpu_SGX		(1u << 2)
#define bit_cpu_BMI1		(1u << 3)
#define bit_cpu_HLE		(1u << 4)
#define bit_cpu_AVX2		(1u << 5)
#define bit_cpu_SMEP		(1u << 7)
#define bit_cpu_BMI2		(1u << 8)
#define bit_cpu_ERMS		(1u << 9)
#define bit_cpu_INVPCID		(1u << 10)
#define bit_cpu_RTM		(1u << 11)
#define bit_cpu_PQM		(1u << 12)
#define bit_cpu_MPX		(1u << 14)
#define bit_cpu_PQE		(1u << 15)
#define bit_cpu_AVX512F		(1u << 16)
#define bit_cpu_AVX512DQ	(1u << 17)
#define bit_cpu_RDSEED		(1u << 18)
#define bit_cpu_ADX		(1u << 19)
#define bit_cpu_SMAP		(1u << 20)
#define bit_cpu_AVX512_IFMA	(1u << 21)
#define bit_cpu_CLFLUSHOPT	(1u << 22)
#define bit_cpu_CLWB		(1u << 24)
#define bit_cpu_TRACE		(1u << 25)
#define bit_cpu_AVX512PF	(1u << 26)
#define bit_cpu_AVX512ER	(1u << 27)
#define bit_cpu_AVX512CD	(1u << 28)
#define bit_cpu_SHA		(1u << 29)
#define bit_cpu_AVX512BW	(1u << 30)
#define bit_cpu_AVX512VL	(1u << 31)

/* ECX.  */
#define bit_cpu_PREFETCHWT1	(1u << 0)
#define bit_cpu_AVX512_VBMI	(1u << 1)
#define bit_cpu_UMIP		(1u << 2)
#define bit_cpu_PKU		(1u << 3)
#define bit_cpu_OSPKE		(1u << 4)
#define bit_cpu_WAITPKG		(1u << 5)
#define bit_cpu_AVX512_VBMI2	(1u << 6)
#define bit_cpu_SHSTK		(1u << 7)
#define bit_cpu_GFNI		(1u << 8)
#define bit_cpu_VAES		(1u << 9)
#define bit_cpu_VPCLMULQDQ	(1u << 10)
#define bit_cpu_AVX512_VNNI	(1u << 11)
#define bit_cpu_AVX512_BITALG	(1u << 12)
#define bit_cpu_AVX512_VPOPCNTDQ (1u << 14)
#define bit_cpu_RDPID		(1u << 22)
#define bit_cpu_CLDEMOTE	(1u << 25)
#define bit_cpu_MOVDIRI		(1u << 27)
#define bit_cpu_MOVDIR64B	(1u << 28)
#define bit_cpu_SGX_LC		(1u << 30)

/* EDX.  */
#define bit_cpu_AVX512_4VNNIW	(1u << 2)
#define bit_cpu_AVX512_4FMAPS	(1u << 3)
#define bit_cpu_FSRM		(1u << 4)
#define bit_cpu_PCONFIG		(1u << 18)
#define bit_cpu_IBT		(1u << 20)
#define bit_cpu_IBRS_IBPB	(1u << 26)
#define bit_cpu_STIBP		(1u << 27)
#define bit_cpu_CAPABILITIES	(1u << 29)
#define bit_cpu_SSBD		(1u << 31)

/* COMMON_CPUID_INDEX_80000001.  */

/* ECX.  */
#define bit_cpu_LAHF64_SAHF64	(1u << 0)
#define bit_cpu_SVM		(1u << 2)
#define bit_cpu_LZCNT		(1u << 5)
#define bit_cpu_SSE4A		(1u << 6)
#define bit_cpu_PREFETCHW	(1u << 8)
#define bit_cpu_XOP		(1u << 11)
#define bit_cpu_LWP		(1u << 15)
#define bit_cpu_FMA4		(1u << 16)
#define bit_cpu_TBM		(1u << 21)

/* EDX.  */
#define bit_cpu_SYSCALL_SYSRET	(1u << 11)
#define bit_cpu_NX		(1u << 20)
#define bit_cpu_PAGE1GB		(1u << 26)
#define bit_cpu_RDTSCP		(1u << 27)
#define bit_cpu_LM		(1u << 29)

/* COMMON_CPUID_INDEX_D_ECX_1.  */

/* EAX.  */
#define bit_cpu_XSAVEOPT	(1u << 0)
#define bit_cpu_XSAVEC		(1u << 1)
#define bit_cpu_XGETBV_ECX_1	(1u << 2)
#define bit_cpu_XSAVES		(1u << 3)

/* COMMON_CPUID_INDEX_80000007.  */

/* EDX.  */
#define bit_cpu_INVARIANT_TSC	(1u << 8)

/* COMMON_CPUID_INDEX_80000008.  */

/* EBX.  */
#define bit_cpu_WBNOINVD	(1u << 9)

/* COMMON_CPUID_INDEX_1.  */

/* ECX.  */
#define index_cpu_SSE3		COMMON_CPUID_INDEX_1
#define index_cpu_PCLMULQDQ	COMMON_CPUID_INDEX_1
#define index_cpu_DTES64	COMMON_CPUID_INDEX_1
#define index_cpu_MONITOR	COMMON_CPUID_INDEX_1
#define index_cpu_DS_CPL	COMMON_CPUID_INDEX_1
#define index_cpu_VMX		COMMON_CPUID_INDEX_1
#define index_cpu_SMX		COMMON_CPUID_INDEX_1
#define index_cpu_EST		COMMON_CPUID_INDEX_1
#define index_cpu_TM2		COMMON_CPUID_INDEX_1
#define index_cpu_SSSE3		COMMON_CPUID_INDEX_1
#define index_cpu_CNXT_ID	COMMON_CPUID_INDEX_1
#define index_cpu_SDBG		COMMON_CPUID_INDEX_1
#define index_cpu_FMA		COMMON_CPUID_INDEX_1
#define index_cpu_CMPXCHG16B	COMMON_CPUID_INDEX_1
#define index_cpu_XTPRUPDCTRL	COMMON_CPUID_INDEX_1
#define index_cpu_PDCM		COMMON_CPUID_INDEX_1
#define index_cpu_PCID		COMMON_CPUID_INDEX_1
#define index_cpu_DCA		COMMON_CPUID_INDEX_1
#define index_cpu_SSE4_1	COMMON_CPUID_INDEX_1
#define index_cpu_SSE4_2	COMMON_CPUID_INDEX_1
#define index_cpu_X2APIC	COMMON_CPUID_INDEX_1
#define index_cpu_MOVBE		COMMON_CPUID_INDEX_1
#define index_cpu_POPCNT	COMMON_CPUID_INDEX_1
#define index_cpu_TSC_DEADLINE	COMMON_CPUID_INDEX_1
#define index_cpu_AES		COMMON_CPUID_INDEX_1
#define index_cpu_XSAVE		COMMON_CPUID_INDEX_1
#define index_cpu_OSXSAVE	COMMON_CPUID_INDEX_1
#define index_cpu_AVX		COMMON_CPUID_INDEX_1
#define index_cpu_F16C		COMMON_CPUID_INDEX_1
#define index_cpu_RDRAND	COMMON_CPUID_INDEX_1

/* ECX.  */
#define index_cpu_FPU		COMMON_CPUID_INDEX_1
#define index_cpu_VME		COMMON_CPUID_INDEX_1
#define index_cpu_DE		COMMON_CPUID_INDEX_1
#define index_cpu_PSE		COMMON_CPUID_INDEX_1
#define index_cpu_TSC		COMMON_CPUID_INDEX_1
#define index_cpu_MSR		COMMON_CPUID_INDEX_1
#define index_cpu_PAE		COMMON_CPUID_INDEX_1
#define index_cpu_MCE		COMMON_CPUID_INDEX_1
#define index_cpu_CX8		COMMON_CPUID_INDEX_1
#define index_cpu_APIC		COMMON_CPUID_INDEX_1
#define index_cpu_SEP		COMMON_CPUID_INDEX_1
#define index_cpu_MTRR		COMMON_CPUID_INDEX_1
#define index_cpu_PGE		COMMON_CPUID_INDEX_1
#define index_cpu_MCA		COMMON_CPUID_INDEX_1
#define index_cpu_CMOV		COMMON_CPUID_INDEX_1
#define index_cpu_PAT		COMMON_CPUID_INDEX_1
#define index_cpu_PSE_36	COMMON_CPUID_INDEX_1
#define index_cpu_PSN		COMMON_CPUID_INDEX_1
#define index_cpu_CLFSH		COMMON_CPUID_INDEX_1
#define index_cpu_DS		COMMON_CPUID_INDEX_1
#define index_cpu_ACPI		COMMON_CPUID_INDEX_1
#define index_cpu_MMX		COMMON_CPUID_INDEX_1
#define index_cpu_FXSR		COMMON_CPUID_INDEX_1
#define index_cpu_SSE		COMMON_CPUID_INDEX_1
#define index_cpu_SSE2		COMMON_CPUID_INDEX_1
#define index_cpu_SS		COMMON_CPUID_INDEX_1
#define index_cpu_HTT		COMMON_CPUID_INDEX_1
#define index_cpu_TM		COMMON_CPUID_INDEX_1
#define index_cpu_PBE		COMMON_CPUID_INDEX_1

/* COMMON_CPUID_INDEX_7.  */

/* EBX.  */
#define index_cpu_FSGSBASE	COMMON_CPUID_INDEX_7
#define index_cpu_TSC_ADJUST	COMMON_CPUID_INDEX_7
#define index_cpu_SGX		COMMON_CPUID_INDEX_7
#define index_cpu_BMI1		COMMON_CPUID_INDEX_7
#define index_cpu_HLE		COMMON_CPUID_INDEX_7
#define index_cpu_AVX2		COMMON_CPUID_INDEX_7
#define index_cpu_SMEP		COMMON_CPUID_INDEX_7
#define index_cpu_BMI2		COMMON_CPUID_INDEX_7
#define index_cpu_ERMS		COMMON_CPUID_INDEX_7
#define index_cpu_INVPCID	COMMON_CPUID_INDEX_7
#define index_cpu_RTM		COMMON_CPUID_INDEX_7
#define index_cpu_PQM		COMMON_CPUID_INDEX_7
#define index_cpu_MPX		COMMON_CPUID_INDEX_7
#define index_cpu_PQE		COMMON_CPUID_INDEX_7
#define index_cpu_AVX512F	COMMON_CPUID_INDEX_7
#define index_cpu_AVX512DQ	COMMON_CPUID_INDEX_7
#define index_cpu_RDSEED	COMMON_CPUID_INDEX_7
#define index_cpu_ADX		COMMON_CPUID_INDEX_7
#define index_cpu_SMAP		COMMON_CPUID_INDEX_7
#define index_cpu_AVX512_IFMA	COMMON_CPUID_INDEX_7
#define index_cpu_CLFLUSHOPT	COMMON_CPUID_INDEX_7
#define index_cpu_CLWB		COMMON_CPUID_INDEX_7
#define index_cpu_TRACE		COMMON_CPUID_INDEX_7
#define index_cpu_AVX512PF	COMMON_CPUID_INDEX_7
#define index_cpu_AVX512ER	COMMON_CPUID_INDEX_7
#define index_cpu_AVX512CD	COMMON_CPUID_INDEX_7
#define index_cpu_SHA		COMMON_CPUID_INDEX_7
#define index_cpu_AVX512BW	COMMON_CPUID_INDEX_7
#define index_cpu_AVX512VL	COMMON_CPUID_INDEX_7

/* ECX.  */
#define index_cpu_PREFETCHWT1	COMMON_CPUID_INDEX_7
#define index_cpu_AVX512_VBMI	COMMON_CPUID_INDEX_7
#define index_cpu_UMIP		COMMON_CPUID_INDEX_7
#define index_cpu_PKU		COMMON_CPUID_INDEX_7
#define index_cpu_OSPKE		COMMON_CPUID_INDEX_7
#define index_cpu_WAITPKG	COMMON_CPUID_INDEX_7
#define index_cpu_AVX512_VBMI2	COMMON_CPUID_INDEX_7
#define index_cpu_SHSTK		COMMON_CPUID_INDEX_7
#define index_cpu_GFNI		COMMON_CPUID_INDEX_7
#define index_cpu_VAES		COMMON_CPUID_INDEX_7
#define index_cpu_VPCLMULQDQ	COMMON_CPUID_INDEX_7
#define index_cpu_AVX512_VNNI	COMMON_CPUID_INDEX_7
#define index_cpu_AVX512_BITALG COMMON_CPUID_INDEX_7
#define index_cpu_AVX512_VPOPCNTDQ COMMON_CPUID_INDEX_7
#define index_cpu_RDPID		COMMON_CPUID_INDEX_7
#define index_cpu_CLDEMOTE	COMMON_CPUID_INDEX_7
#define index_cpu_MOVDIRI	COMMON_CPUID_INDEX_7
#define index_cpu_MOVDIR64B	COMMON_CPUID_INDEX_7
#define index_cpu_SGX_LC	COMMON_CPUID_INDEX_7

/* EDX.  */
#define index_cpu_AVX512_4VNNIW COMMON_CPUID_INDEX_7
#define index_cpu_AVX512_4FMAPS	COMMON_CPUID_INDEX_7
#define index_cpu_FSRM		COMMON_CPUID_INDEX_7
#define index_cpu_PCONFIG	COMMON_CPUID_INDEX_7
#define index_cpu_IBT		COMMON_CPUID_INDEX_7
#define index_cpu_IBRS_IBPB	COMMON_CPUID_INDEX_7
#define index_cpu_STIBP		COMMON_CPUID_INDEX_7
#define index_cpu_CAPABILITIES	COMMON_CPUID_INDEX_7
#define index_cpu_SSBD		COMMON_CPUID_INDEX_7

/* COMMON_CPUID_INDEX_80000001.  */

/* ECX.  */
#define index_cpu_LAHF64_SAHF64 COMMON_CPUID_INDEX_80000001
#define index_cpu_SVM		COMMON_CPUID_INDEX_80000001
#define index_cpu_LZCNT		COMMON_CPUID_INDEX_80000001
#define index_cpu_SSE4A		COMMON_CPUID_INDEX_80000001
#define index_cpu_PREFETCHW	COMMON_CPUID_INDEX_80000001
#define index_cpu_XOP		COMMON_CPUID_INDEX_80000001
#define index_cpu_LWP		COMMON_CPUID_INDEX_80000001
#define index_cpu_FMA4		COMMON_CPUID_INDEX_80000001
#define index_cpu_TBM		COMMON_CPUID_INDEX_80000001

/* EDX.  */
#define index_cpu_SYSCALL_SYSRET COMMON_CPUID_INDEX_80000001
#define index_cpu_NX		COMMON_CPUID_INDEX_80000001
#define index_cpu_PAGE1GB	COMMON_CPUID_INDEX_80000001
#define index_cpu_RDTSCP	COMMON_CPUID_INDEX_80000001
#define index_cpu_LM		COMMON_CPUID_INDEX_80000001

/* COMMON_CPUID_INDEX_D_ECX_1.  */

/* EAX.  */
#define index_cpu_XSAVEOPT	COMMON_CPUID_INDEX_D_ECX_1
#define index_cpu_XSAVEC	COMMON_CPUID_INDEX_D_ECX_1
#define index_cpu_XGETBV_ECX_1	COMMON_CPUID_INDEX_D_ECX_1
#define index_cpu_XSAVES	COMMON_CPUID_INDEX_D_ECX_1

/* COMMON_CPUID_INDEX_80000007.  */

/* EDX.  */
#define index_cpu_INVARIANT_TSC	COMMON_CPUID_INDEX_80000007

/* COMMON_CPUID_INDEX_80000008.  */

/* EBX.  */
#define index_cpu_WBNOINVD	COMMON_CPUID_INDEX_80000008

/* COMMON_CPUID_INDEX_1.  */

/* ECX.  */
#define reg_SSE3		ecx
#define reg_PCLMULQDQ		ecx
#define reg_DTES64		ecx
#define reg_MONITOR		ecx
#define reg_DS_CPL		ecx
#define reg_VMX			ecx
#define reg_SMX			ecx
#define reg_EST			ecx
#define reg_TM2			ecx
#define reg_SSSE3		ecx
#define reg_CNXT_ID		ecx
#define reg_SDBG		ecx
#define reg_FMA			ecx
#define reg_CMPXCHG16B		ecx
#define reg_XTPRUPDCTRL		ecx
#define reg_PDCM		ecx
#define reg_PCID		ecx
#define reg_DCA			ecx
#define reg_SSE4_1		ecx
#define reg_SSE4_2		ecx
#define reg_X2APIC		ecx
#define reg_MOVBE		ecx
#define reg_POPCNT		ecx
#define reg_TSC_DEADLINE	ecx
#define reg_AES			ecx
#define reg_XSAVE		ecx
#define reg_OSXSAVE		ecx
#define reg_AVX			ecx
#define reg_F16C		ecx
#define reg_RDRAND		ecx

/* EDX.  */
#define reg_FPU			edx
#define reg_VME			edx
#define reg_DE			edx
#define reg_PSE			edx
#define reg_TSC			edx
#define reg_MSR			edx
#define reg_PAE			edx
#define reg_MCE			edx
#define reg_CX8			edx
#define reg_APIC		edx
#define reg_SEP			edx
#define reg_MTRR		edx
#define reg_PGE			edx
#define reg_MCA			edx
#define reg_CMOV		edx
#define reg_PAT			edx
#define reg_PSE_36		edx
#define reg_PSN			edx
#define reg_CLFSH		edx
#define reg_DS			edx
#define reg_ACPI		edx
#define reg_MMX			edx
#define reg_FXSR		edx
#define reg_SSE			edx
#define reg_SSE2		edx
#define reg_SS			edx
#define reg_HTT			edx
#define reg_TM			edx
#define reg_PBE			edx

/* COMMON_CPUID_INDEX_7.  */

/* EBX.  */
#define reg_FSGSBASE		ebx
#define reg_TSC_ADJUST		ebx
#define reg_SGX			ebx
#define reg_BMI1		ebx
#define reg_HLE			ebx
#define reg_BMI2		ebx
#define reg_AVX2		ebx
#define reg_SMEP		ebx
#define reg_ERMS		ebx
#define reg_INVPCID		ebx
#define reg_RTM			ebx
#define reg_PQM			ebx
#define reg_MPX			ebx
#define reg_PQE			ebx
#define reg_AVX512F		ebx
#define reg_AVX512DQ		ebx
#define reg_RDSEED		ebx
#define reg_ADX			ebx
#define reg_SMAP		ebx
#define reg_AVX512_IFMA		ebx
#define reg_CLFLUSHOPT		ebx
#define reg_CLWB		ebx
#define reg_TRACE		ebx
#define reg_AVX512PF		ebx
#define reg_AVX512ER		ebx
#define reg_AVX512CD		ebx
#define reg_SHA			ebx
#define reg_AVX512BW		ebx
#define reg_AVX512VL		ebx

/* ECX.  */
#define reg_PREFETCHWT1		ecx
#define reg_AVX512_VBMI		ecx
#define reg_UMIP		ecx
#define reg_PKU			ecx
#define reg_OSPKE		ecx
#define reg_WAITPKG		ecx
#define reg_AVX512_VBMI2	ecx
#define reg_SHSTK		ecx
#define reg_GFNI		ecx
#define reg_VAES		ecx
#define reg_VPCLMULQDQ		ecx
#define reg_AVX512_VNNI		ecx
#define reg_AVX512_BITALG	ecx
#define reg_AVX512_VPOPCNTDQ	ecx
#define reg_RDPID		ecx
#define reg_CLDEMOTE		ecx
#define reg_MOVDIRI		ecx
#define reg_MOVDIR64B		ecx
#define reg_SGX_LC		ecx

/* EDX.  */
#define reg_AVX512_4VNNIW	edx
#define reg_AVX512_4FMAPS	edx
#define reg_FSRM		edx
#define reg_PCONFIG		edx
#define reg_IBT			edx
#define reg_IBRS_IBPB		edx
#define reg_STIBP		edx
#define reg_CAPABILITIES	edx
#define reg_SSBD		edx

/* COMMON_CPUID_INDEX_80000001.  */

/* ECX.  */
#define reg_LAHF64_SAHF64	ecx
#define reg_SVM			ecx
#define reg_LZCNT		ecx
#define reg_SSE4A		ecx
#define reg_PREFETCHW		ecx
#define reg_XOP			ecx
#define reg_LWP			ecx
#define reg_FMA4		ecx
#define reg_TBM			ecx

/* EDX.  */
#define reg_SYSCALL_SYSRET	edx
#define reg_NX			edx
#define reg_PAGE1GB		edx
#define reg_RDTSCP		edx
#define reg_LM			edx

/* COMMON_CPUID_INDEX_D_ECX_1.  */

/* EAX.  */
#define reg_XSAVEOPT		eax
#define reg_XSAVEC		eax
#define reg_XGETBV_ECX_1	eax
#define reg_XSAVES		eax

/* COMMON_CPUID_INDEX_80000007.  */

/* EDX.  */
#define reg_INVARIANT_TSC	edx

/* COMMON_CPUID_INDEX_80000008.  */

/* EBX.  */
#define reg_WBNOINVD		ebx

/* FEATURE_INDEX_2.  */
#define bit_arch_I586				(1u << 0)
#define bit_arch_I686				(1u << 1)
#define bit_arch_Fast_Rep_String		(1u << 2)
#define bit_arch_Fast_Copy_Backward		(1u << 3)
#define bit_arch_Fast_Unaligned_Load		(1u << 4)
#define bit_arch_Fast_Unaligned_Copy		(1u << 5)
#define bit_arch_Slow_BSF			(1u << 6)
#define bit_arch_Slow_SSE4_2			(1u << 7)
#define bit_arch_AVX_Fast_Unaligned_Load	(1u << 8)
#define bit_arch_Prefer_MAP_32BIT_EXEC		(1u << 9)
#define bit_arch_Prefer_PMINUB_for_stringop	(1u << 10)
#define bit_arch_Prefer_No_VZEROUPPER		(1u << 11)
#define bit_arch_Prefer_ERMS			(1u << 12)
#define bit_arch_Prefer_FSRM			(1u << 13)
#define bit_arch_Prefer_No_AVX512		(1u << 14)
#define bit_arch_MathVec_Prefer_No_AVX512	(1u << 15)

#define index_arch_Fast_Rep_String		FEATURE_INDEX_2
#define index_arch_Fast_Copy_Backward		FEATURE_INDEX_2
#define index_arch_Slow_BSF			FEATURE_INDEX_2
#define index_arch_Fast_Unaligned_Load		FEATURE_INDEX_2
#define index_arch_Prefer_PMINUB_for_stringop 	FEATURE_INDEX_2
#define index_arch_Fast_Unaligned_Copy		FEATURE_INDEX_2
#define index_arch_I586				FEATURE_INDEX_2
#define index_arch_I686				FEATURE_INDEX_2
#define index_arch_Slow_SSE4_2			FEATURE_INDEX_2
#define index_arch_AVX_Fast_Unaligned_Load	FEATURE_INDEX_2
#define index_arch_Prefer_MAP_32BIT_EXEC	FEATURE_INDEX_2
#define index_arch_Prefer_No_VZEROUPPER		FEATURE_INDEX_2
#define index_arch_Prefer_ERMS			FEATURE_INDEX_2
#define index_arch_Prefer_No_AVX512		FEATURE_INDEX_2
#define index_arch_MathVec_Prefer_No_AVX512	FEATURE_INDEX_2
#define index_arch_Prefer_FSRM			FEATURE_INDEX_2

/* XCR0 Feature flags.  */
#define bit_XMM_state		(1u << 1)
#define bit_YMM_state		(1u << 2)
#define bit_Opmask_state	(1u << 5)
#define bit_ZMM0_15_state	(1u << 6)
#define bit_ZMM16_31_state	(1u << 7)

# if defined (_LIBC) && !IS_IN (nonlib)
/* Unused for x86.  */
#  define INIT_ARCH()
#  define __get_cpu_features()	(&GLRO(dl_x86_cpu_features))
#  define x86_get_cpuid_registers(i) \
       (&(GLRO(dl_x86_cpu_features).cpuid[i]))
# endif

#ifdef __x86_64__
# define HAS_CPUID 1
#elif (defined __i586__ || defined __pentium__	\
       || defined __geode__ || defined __k6__)
# define HAS_CPUID 1
# define HAS_I586 1
# define HAS_I686 HAS_ARCH_FEATURE (I686)
#elif defined __i486__
# define HAS_CPUID 0
# define HAS_I586 HAS_ARCH_FEATURE (I586)
# define HAS_I686 HAS_ARCH_FEATURE (I686)
#else
# define HAS_CPUID 1
# define HAS_I586 1
# define HAS_I686 1
#endif

#endif  /* cpu_features_h */
