/* x86 cache info.
   Copyright (C) 2003-2020 Free Software Foundation, Inc.
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

#if IS_IN (libc)

#include <assert.h>
#include <unistd.h>
#include <cpuid.h>
#include <ldsodefs.h>
#include <dl-cacheinfo.h>

/* Get the value of the system variable NAME.  */
long int
attribute_hidden
__cache_sysconf (int name)
{
  const struct cpu_features *cpu_features = __get_cpu_features ();

  if (cpu_features->basic.kind == arch_kind_intel)
    return handle_intel (name, cpu_features);

  if (cpu_features->basic.kind == arch_kind_amd)
    return handle_amd (name);

  if (cpu_features->basic.kind == arch_kind_zhaoxin)
    return handle_zhaoxin (name);

  // XXX Fill in more vendors.

  /* CPU not known, we have no information.  */
  return 0;
}

# ifdef SHARED
/* NB: In libc.a, cacheinfo.h is included in libc-start.c.  In libc.so,
   cacheinfo.h is included here and call init_cacheinfo by initializing
   a dummy function pointer via IFUNC relocation after CPU features in
   ld.so have been initialized by DL_PLATFORM_INIT or IFUNC relocation.  */
# include <cacheinfo.h>
# include <ifunc-init.h>

extern void __x86_cacheinfo (void) attribute_hidden;
const void (*__x86_cacheinfo_p) (void) attribute_hidden
  = __x86_cacheinfo;

__ifunc (__x86_cacheinfo, __x86_cacheinfo, NULL, void, init_cacheinfo);
# endif
#endif
