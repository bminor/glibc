/* x86 cache info.
   Copyright (C) 2020-2021 Free Software Foundation, Inc.
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
#include <cpuid.h>
#include <cpu-features.h>

#if HAVE_TUNABLES
# define TUNABLE_NAMESPACE cpu
# include <unistd.h>		/* Get STDOUT_FILENO for _dl_printf.  */
# include <elf/dl-tunables.h>
#endif

#if IS_IN (libc)
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
init_cacheinfo (void)
{
  const struct cpu_features *cpu_features = __get_cpu_features ();
  long int data = cpu_features->data_cache_size;
  __x86_raw_data_cache_size_half = data / 2;
  __x86_raw_data_cache_size = data;
  /* Round data cache size to multiple of 256 bytes.  */
  data = data & ~255L;
  __x86_data_cache_size_half = data / 2;
  __x86_data_cache_size = data;

  long int shared = cpu_features->shared_cache_size;
  __x86_raw_shared_cache_size_half = shared / 2;
  __x86_raw_shared_cache_size = shared;
  /* Round shared cache size to multiple of 256 bytes.  */
  shared = shared & ~255L;
  __x86_shared_cache_size_half = shared / 2;
  __x86_shared_cache_size = shared;

  __x86_shared_non_temporal_threshold
    = cpu_features->non_temporal_threshold;

  __x86_rep_movsb_threshold = cpu_features->rep_movsb_threshold;
  __x86_rep_stosb_threshold = cpu_features->rep_stosb_threshold;
}
#endif
