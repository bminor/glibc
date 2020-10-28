/* x86 cache info.
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

#include <assert.h>
#include <unistd.h>

/* Data cache size for use in memory and string routines, typically
   L1 size, rounded to multiple of 256 bytes.  */
long int __x86_data_cache_size_half attribute_hidden = 32 * 1024 / 2;
long int __x86_data_cache_size attribute_hidden = 32 * 1024;
/* Similar to __x86_data_cache_size_half, but not rounded.  */
long int __x86_raw_data_cache_size_half attribute_hidden = 32 * 1024 / 2;
/* Similar to __x86_data_cache_size, but not rounded.  */
long int __x86_raw_data_cache_size attribute_hidden = 32 * 1024;
/* Shared cache size for use in memory and string routines, typically
   L2 or L3 size, rounded to multiple of 256 bytes.  */
long int __x86_shared_cache_size_half attribute_hidden = 1024 * 1024 / 2;
long int __x86_shared_cache_size attribute_hidden = 1024 * 1024;
/* Similar to __x86_shared_cache_size_half, but not rounded.  */
long int __x86_raw_shared_cache_size_half attribute_hidden = 1024 * 1024 / 2;
/* Similar to __x86_shared_cache_size, but not rounded.  */
long int __x86_raw_shared_cache_size attribute_hidden = 1024 * 1024;

/* Threshold to use non temporal store.  */
long int __x86_shared_non_temporal_threshold attribute_hidden;

/* Threshold to use Enhanced REP MOVSB.  */
long int __x86_rep_movsb_threshold attribute_hidden = 2048;

/* Threshold to use Enhanced REP STOSB.  */
long int __x86_rep_stosb_threshold attribute_hidden = 2048;

static void
get_common_cache_info (long int *shared_ptr, unsigned int *threads_ptr,
		       long int core)
{
  unsigned int eax;
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;

  /* Number of logical processors sharing L2 cache.  */
  int threads_l2;

  /* Number of logical processors sharing L3 cache.  */
  int threads_l3;

  const struct cpu_features *cpu_features = __get_cpu_features ();
  int max_cpuid = cpu_features->basic.max_cpuid;
  unsigned int family = cpu_features->basic.family;
  unsigned int model = cpu_features->basic.model;
  long int shared = *shared_ptr;
  unsigned int threads = *threads_ptr;
  bool inclusive_cache = true;
  bool support_count_mask = true;

  /* Try L3 first.  */
  unsigned int level = 3;

  if (cpu_features->basic.kind == arch_kind_zhaoxin && family == 6)
    support_count_mask = false;

  if (shared <= 0)
    {
      /* Try L2 otherwise.  */
      level  = 2;
      shared = core;
      threads_l2 = 0;
      threads_l3 = -1;
    }
  else
    {
      threads_l2 = 0;
      threads_l3 = 0;
    }

  /* A value of 0 for the HTT bit indicates there is only a single
     logical processor.  */
  if (HAS_CPU_FEATURE (HTT))
    {
      /* Figure out the number of logical threads that share the
         highest cache level.  */
      if (max_cpuid >= 4)
        {
          int i = 0;

          /* Query until cache level 2 and 3 are enumerated.  */
          int check = 0x1 | (threads_l3 == 0) << 1;
          do
            {
              __cpuid_count (4, i++, eax, ebx, ecx, edx);

              /* There seems to be a bug in at least some Pentium Ds
                 which sometimes fail to iterate all cache parameters.
                 Do not loop indefinitely here, stop in this case and
                 assume there is no such information.  */
              if (cpu_features->basic.kind == arch_kind_intel
                  && (eax & 0x1f) == 0 )
                goto intel_bug_no_cache_info;

              switch ((eax >> 5) & 0x7)
                {
                  default:
                    break;
                  case 2:
                    if ((check & 0x1))
                      {
                        /* Get maximum number of logical processors
                           sharing L2 cache.  */
                        threads_l2 = (eax >> 14) & 0x3ff;
                        check &= ~0x1;
                      }
                    break;
                  case 3:
                    if ((check & (0x1 << 1)))
                      {
                        /* Get maximum number of logical processors
                           sharing L3 cache.  */
                        threads_l3 = (eax >> 14) & 0x3ff;

                        /* Check if L2 and L3 caches are inclusive.  */
                        inclusive_cache = (edx & 0x2) != 0;
                        check &= ~(0x1 << 1);
                      }
                    break;
                }
            }
          while (check);

          /* If max_cpuid >= 11, THREADS_L2/THREADS_L3 are the maximum
             numbers of addressable IDs for logical processors sharing
             the cache, instead of the maximum number of threads
             sharing the cache.  */
          if (max_cpuid >= 11 && support_count_mask)
            {
              /* Find the number of logical processors shipped in
                 one core and apply count mask.  */
              i = 0;

              /* Count SMT only if there is L3 cache.  Always count
                 core if there is no L3 cache.  */
              int count = ((threads_l2 > 0 && level == 3)
                           | ((threads_l3 > 0
                               || (threads_l2 > 0 && level == 2)) << 1));

              while (count)
                {
                  __cpuid_count (11, i++, eax, ebx, ecx, edx);

                  int shipped = ebx & 0xff;
                  int type = ecx & 0xff00;
                  if (shipped == 0 || type == 0)
                    break;
                  else if (type == 0x100)
                    {
                      /* Count SMT.  */
                      if ((count & 0x1))
                        {
                          int count_mask;

                          /* Compute count mask.  */
                          asm ("bsr %1, %0"
                               : "=r" (count_mask) : "g" (threads_l2));
                          count_mask = ~(-1 << (count_mask + 1));
                          threads_l2 = (shipped - 1) & count_mask;
                          count &= ~0x1;
                        }
                    }
                  else if (type == 0x200)
                    {
                      /* Count core.  */
                      if ((count & (0x1 << 1)))
                        {
                          int count_mask;
                          int threads_core
                            = (level == 2 ? threads_l2 : threads_l3);

                          /* Compute count mask.  */
                          asm ("bsr %1, %0"
                               : "=r" (count_mask) : "g" (threads_core));
                          count_mask = ~(-1 << (count_mask + 1));
                          threads_core = (shipped - 1) & count_mask;
                          if (level == 2)
                            threads_l2 = threads_core;
                          else
                            threads_l3 = threads_core;
                          count &= ~(0x1 << 1);
                        }
                    }
                }
            }
          if (threads_l2 > 0)
            threads_l2 += 1;
          if (threads_l3 > 0)
            threads_l3 += 1;
          if (level == 2)
            {
              if (threads_l2)
                {
                  threads = threads_l2;
                  if (cpu_features->basic.kind == arch_kind_intel
                      && threads > 2
                      && family == 6)
                    switch (model)
                      {
                        case 0x37:
                        case 0x4a:
                        case 0x4d:
                        case 0x5a:
                        case 0x5d:
                          /* Silvermont has L2 cache shared by 2 cores.  */
                          threads = 2;
                          break;
                        default:
                          break;
                      }
                }
            }
          else if (threads_l3)
            threads = threads_l3;
        }
      else
        {
intel_bug_no_cache_info:
          /* Assume that all logical threads share the highest cache
             level.  */
          threads
            = ((cpu_features->features[COMMON_CPUID_INDEX_1].cpuid.ebx
                >> 16) & 0xff);
        }

        /* Cap usage of highest cache level to the number of supported
           threads.  */
        if (shared > 0 && threads > 0)
          shared /= threads;
    }

  /* Account for non-inclusive L2 and L3 caches.  */
  if (!inclusive_cache)
    {
      if (threads_l2 > 0)
        core /= threads_l2;
      shared += core;
    }

  *shared_ptr = shared;
  *threads_ptr = threads;
}

static void
init_cacheinfo (void)
{
  /* Find out what brand of processor.  */
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;
  int max_cpuid_ex;
  long int data = -1;
  long int shared = -1;
  long int core;
  unsigned int threads = 0;
  const struct cpu_features *cpu_features = __get_cpu_features ();

  /* NB: In libc.so, cpu_features is defined in ld.so and is initialized
     by DL_PLATFORM_INIT or IFUNC relocation before init_cacheinfo is
     called by IFUNC relocation.  In libc.a, init_cacheinfo is called
     from init_cpu_features by ARCH_INIT_CPU_FEATURES.  */
  assert (cpu_features->basic.kind != arch_kind_unknown);

  if (cpu_features->basic.kind == arch_kind_intel)
    {
      data = handle_intel (_SC_LEVEL1_DCACHE_SIZE, cpu_features);
      core = handle_intel (_SC_LEVEL2_CACHE_SIZE, cpu_features);
      shared = handle_intel (_SC_LEVEL3_CACHE_SIZE, cpu_features);

      get_common_cache_info (&shared, &threads, core);
    }
  else if (cpu_features->basic.kind == arch_kind_zhaoxin)
    {
      data = handle_zhaoxin (_SC_LEVEL1_DCACHE_SIZE);
      core = handle_zhaoxin (_SC_LEVEL2_CACHE_SIZE);
      shared = handle_zhaoxin (_SC_LEVEL3_CACHE_SIZE);

      get_common_cache_info (&shared, &threads, core);
    }
  else if (cpu_features->basic.kind == arch_kind_amd)
    {
      data   = handle_amd (_SC_LEVEL1_DCACHE_SIZE);
      long int core = handle_amd (_SC_LEVEL2_CACHE_SIZE);
      shared = handle_amd (_SC_LEVEL3_CACHE_SIZE);

      /* Get maximum extended function. */
      __cpuid (0x80000000, max_cpuid_ex, ebx, ecx, edx);

      if (shared <= 0)
	/* No shared L3 cache.  All we have is the L2 cache.  */
	shared = core;
      else
	{
	  /* Figure out the number of logical threads that share L3.  */
	  if (max_cpuid_ex >= 0x80000008)
	    {
	      /* Get width of APIC ID.  */
	      __cpuid (0x80000008, max_cpuid_ex, ebx, ecx, edx);
	      threads = 1 << ((ecx >> 12) & 0x0f);
	    }

	  if (threads == 0 || cpu_features->basic.family >= 0x17)
	    {
	      /* If APIC ID width is not available, use logical
		 processor count.  */
	      __cpuid (0x00000001, max_cpuid_ex, ebx, ecx, edx);

	      if ((edx & (1 << 28)) != 0)
		threads = (ebx >> 16) & 0xff;
	    }

	  /* Cap usage of highest cache level to the number of
	     supported threads.  */
	  if (threads > 0)
	    shared /= threads;

	  /* Get shared cache per ccx for Zen architectures.  */
	  if (cpu_features->basic.family >= 0x17)
	    {
	      unsigned int eax;

	      /* Get number of threads share the L3 cache in CCX.  */
	      __cpuid_count (0x8000001D, 0x3, eax, ebx, ecx, edx);

	      unsigned int threads_per_ccx = ((eax >> 14) & 0xfff) + 1;
	      shared *= threads_per_ccx;
	    }
	  else
	    {
	      /* Account for exclusive L2 and L3 caches.  */
	      shared += core;
            }
      }
    }

  /* Prefer cache size configure via tuning.  */
  if (cpu_features->data_cache_size != 0)
    data = cpu_features->data_cache_size;

  if (data > 0)
    {
      __x86_raw_data_cache_size_half = data / 2;
      __x86_raw_data_cache_size = data;
      /* Round data cache size to multiple of 256 bytes.  */
      data = data & ~255L;
      __x86_data_cache_size_half = data / 2;
      __x86_data_cache_size = data;
    }

  /* Prefer cache size configure via tuning.  */
  if (cpu_features->shared_cache_size != 0)
    shared = cpu_features->shared_cache_size;

  if (shared > 0)
    {
      __x86_raw_shared_cache_size_half = shared / 2;
      __x86_raw_shared_cache_size = shared;
      /* Round shared cache size to multiple of 256 bytes.  */
      shared = shared & ~255L;
      __x86_shared_cache_size_half = shared / 2;
      __x86_shared_cache_size = shared;
    }

  /* The default setting for the non_temporal threshold is 3/4 of one
     thread's share of the chip's cache. For most Intel and AMD processors
     with an initial release date between 2017 and 2020, a thread's typical
     share of the cache is from 500 KBytes to 2 MBytes. Using the 3/4
     threshold leaves 125 KBytes to 500 KBytes of the thread's data
     in cache after a maximum temporal copy, which will maintain
     in cache a reasonable portion of the thread's stack and other
     active data. If the threshold is set higher than one thread's
     share of the cache, it has a substantial risk of negatively
     impacting the performance of other threads running on the chip. */
  __x86_shared_non_temporal_threshold
    = (cpu_features->non_temporal_threshold != 0
       ? cpu_features->non_temporal_threshold
       : __x86_shared_cache_size * 3 / 4);

  /* NB: The REP MOVSB threshold must be greater than VEC_SIZE * 8.  */
  unsigned int minimum_rep_movsb_threshold;
  /* NB: The default REP MOVSB threshold is 2048 * (VEC_SIZE / 16).  */
  unsigned int rep_movsb_threshold;
  if (CPU_FEATURE_USABLE_P (cpu_features, AVX512F)
      && !CPU_FEATURE_PREFERRED_P (cpu_features, Prefer_No_AVX512))
    {
      rep_movsb_threshold = 2048 * (64 / 16);
      minimum_rep_movsb_threshold = 64 * 8;
    }
  else if (CPU_FEATURE_PREFERRED_P (cpu_features,
				    AVX_Fast_Unaligned_Load))
    {
      rep_movsb_threshold = 2048 * (32 / 16);
      minimum_rep_movsb_threshold = 32 * 8;
    }
  else
    {
      rep_movsb_threshold = 2048 * (16 / 16);
      minimum_rep_movsb_threshold = 16 * 8;
    }
  if (cpu_features->rep_movsb_threshold > minimum_rep_movsb_threshold)
    __x86_rep_movsb_threshold = cpu_features->rep_movsb_threshold;
  else
    __x86_rep_movsb_threshold = rep_movsb_threshold;

# if HAVE_TUNABLES
  __x86_rep_stosb_threshold = cpu_features->rep_stosb_threshold;
# endif
}
