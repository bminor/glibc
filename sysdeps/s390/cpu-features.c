/* Initialize cpu feature data.  s390x version.
   Copyright (C) 2023 Free Software Foundation, Inc.
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

#include <cpu-features.h>

#include <elf/dl-tunables.h>
#include <ifunc-memcmp.h>
#include <string.h>
extern __typeof (memcmp) MEMCMP_DEFAULT;

#define S390_COPY_CPU_FEATURES(SRC_PTR, DEST_PTR)	\
  (DEST_PTR)->hwcap = (SRC_PTR)->hwcap;			\
  (DEST_PTR)->stfle_bits[0] = (SRC_PTR)->stfle_bits[0];

static void
TUNABLE_CALLBACK (set_hwcaps) (tunable_val_t *valp)
{
  /* The current IFUNC selection is always using the most recent
     features which are available via AT_HWCAP or STFLE-bits.  But in
     some scenarios it is useful to adjust this selection.

     The environment variable:

     GLIBC_TUNABLES=glibc.cpu.hwcaps=-xxx,yyy,zzz,....

     can be used to enable HWCAP/STFLE feature yyy, disable HWCAP/STFLE feature
     xxx, where the feature name is case-sensitive and has to match the ones
     used below.  Furthermore, the ARCH-level zzz can be used to set various
     HWCAP/STFLE features at once.  */

  /* Copy the features from dl_s390_cpu_features, which contains the features
     provided by AT_HWCAP and stfle-instruction.  */
  struct cpu_features *cpu_features = &GLRO(dl_s390_cpu_features);
  struct cpu_features cpu_features_orig;
  S390_COPY_CPU_FEATURES (cpu_features, &cpu_features_orig);
  struct cpu_features cpu_features_curr;
  S390_COPY_CPU_FEATURES (cpu_features, &cpu_features_curr);

  const char *token = valp->strval;
  do
    {
      const char *token_end, *feature;
      bool disable;
      size_t token_len;
      size_t feature_len;

      /* Find token separator or end of string.  */
      for (token_end = token; *token_end != ','; token_end++)
	if (*token_end == '\0')
	  break;

      /* Determine feature.  */
      token_len = token_end - token;
      if (*token == '-')
	{
	  disable = true;
	  feature = token + 1;
	  feature_len = token_len - 1;
	}
      else
	{
	  disable = false;
	  feature = token;
	  feature_len = token_len;
	}

      /* Handle only the features here which are really used in the
	 IFUNC-resolvers.  All others are ignored as the values are only used
	 inside glibc.  */
      bool reset_features = false;
      unsigned long int hwcap_mask = 0UL;
      unsigned long long stfle_bits0_mask = 0ULL;

      if ((*feature == 'z' || *feature == 'a'))
	{
	  if ((feature_len == 5 && *feature == 'z'
	       && MEMCMP_DEFAULT (feature, "zEC12", 5) == 0)
	      || (feature_len == 6 && *feature == 'a'
		  && MEMCMP_DEFAULT (feature, "arch10", 6) == 0))
	    {
	      reset_features = true;
	      disable = true;
	      hwcap_mask = HWCAP_S390_VXRS | HWCAP_S390_VXRS_EXT
		| HWCAP_S390_VXRS_EXT2;
	      stfle_bits0_mask = S390_STFLE_MASK_ARCH13_MIE3;
	    }
	  else if ((feature_len == 3 && *feature == 'z'
		    && MEMCMP_DEFAULT (feature, "z13", 3) == 0)
		   || (feature_len == 6 && *feature == 'a'
		       && MEMCMP_DEFAULT (feature, "arch11", 6) == 0))
	    {
	      reset_features = true;
	      disable = true;
	      hwcap_mask = HWCAP_S390_VXRS_EXT | HWCAP_S390_VXRS_EXT2;
	      stfle_bits0_mask = S390_STFLE_MASK_ARCH13_MIE3;
	    }
	  else if ((feature_len == 3 && *feature == 'z'
		    && MEMCMP_DEFAULT (feature, "z14", 3) == 0)
		   || (feature_len == 6 && *feature == 'a'
		       && MEMCMP_DEFAULT (feature, "arch12", 6) == 0))
	    {
	      reset_features = true;
	      disable = true;
	      hwcap_mask = HWCAP_S390_VXRS_EXT2;
	      stfle_bits0_mask = S390_STFLE_MASK_ARCH13_MIE3;
	    }
	  else if ((feature_len == 3 && *feature == 'z'
		    && (MEMCMP_DEFAULT (feature, "z15", 3) == 0
			|| MEMCMP_DEFAULT (feature, "z16", 3) == 0))
		   || (feature_len == 6
		       && (MEMCMP_DEFAULT (feature, "arch13", 6) == 0
			   || MEMCMP_DEFAULT (feature, "arch14", 6) == 0)))
	    {
	      /* For z15 or newer we don't have to disable something,
		 but we have to reset to the original values.  */
	      reset_features = true;
	    }
	}
      else if (*feature == 'H')
	{
	  if (feature_len == 15
	      && MEMCMP_DEFAULT (feature, "HWCAP_S390_VXRS", 15) == 0)
	    {
	      hwcap_mask = HWCAP_S390_VXRS;
	      if (disable)
		hwcap_mask |= HWCAP_S390_VXRS_EXT | HWCAP_S390_VXRS_EXT2;
	    }
	  else if (feature_len == 19
		   && MEMCMP_DEFAULT (feature, "HWCAP_S390_VXRS_EXT", 19) == 0)
	    {
	      hwcap_mask = HWCAP_S390_VXRS_EXT;
	      if (disable)
		hwcap_mask |= HWCAP_S390_VXRS_EXT2;
	      else
		hwcap_mask |= HWCAP_S390_VXRS;
	    }
	  else if (feature_len == 20
		   && MEMCMP_DEFAULT (feature, "HWCAP_S390_VXRS_EXT2", 20) == 0)
	    {
	      hwcap_mask = HWCAP_S390_VXRS_EXT2;
	      if (!disable)
		hwcap_mask |= HWCAP_S390_VXRS | HWCAP_S390_VXRS_EXT;
	    }
	}
      else if (*feature == 'S')
	{
	  if (feature_len == 10
	      && MEMCMP_DEFAULT (feature, "STFLE_MIE3", 10) == 0)
	    {
	      stfle_bits0_mask = S390_STFLE_MASK_ARCH13_MIE3;
	    }
	}

      /* Perform the actions determined above.  */
      if (reset_features)
	{
	  S390_COPY_CPU_FEATURES (&cpu_features_orig, &cpu_features_curr);
	}

      if (hwcap_mask != 0UL)
	{
	  if (disable)
	    cpu_features_curr.hwcap &= ~hwcap_mask;
	  else
	    cpu_features_curr.hwcap |= hwcap_mask;
	}

      if (stfle_bits0_mask != 0ULL)
	{
	  if (disable)
	    cpu_features_curr.stfle_bits[0] &= ~stfle_bits0_mask;
	  else
	    cpu_features_curr.stfle_bits[0] |= stfle_bits0_mask;
	}

      /* Jump over current token ... */
      token += token_len;

      /* ... and skip token separator for next round.  */
      if (*token == ',') token++;
    }
  while (*token != '\0');

  /* Copy back the features after checking that no unsupported features were
     enabled by user.  */
  cpu_features->hwcap = cpu_features_curr.hwcap & cpu_features_orig.hwcap;
  cpu_features->stfle_bits[0] = cpu_features_curr.stfle_bits[0]
    & cpu_features_orig.stfle_bits[0];
}

static inline void
init_cpu_features (struct cpu_features *cpu_features)
{
  /* Fill cpu_features as passed by kernel and machine.  */
  cpu_features->hwcap = GLRO(dl_hwcap);

  /* We want just 1 double word to be returned.  */
  if (__glibc_likely ((cpu_features->hwcap & HWCAP_S390_STFLE)
		      && (cpu_features->hwcap & HWCAP_S390_ZARCH)
		      && (cpu_features->hwcap & HWCAP_S390_HIGH_GPRS)))
    {
      register unsigned long reg0 __asm__("0") = 0;
      __asm__ __volatile__(".machine push"        "\n\t"
			   ".machine \"z9-109\""  "\n\t"
			   ".machinemode \"zarch_nohighgprs\"\n\t"
			   "stfle %0"             "\n\t"
			   ".machine pop"         "\n"
			   : "=QS" (cpu_features->stfle_bits[0]),
			     "+d" (reg0)
			   : : "cc");
    }
  else
    {
      cpu_features->stfle_bits[0] = 0ULL;
    }

  TUNABLE_GET (glibc, cpu, hwcaps, tunable_val_t *, TUNABLE_CALLBACK (set_hwcaps));
}
