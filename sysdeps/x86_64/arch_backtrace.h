/* Architecture-specific backtrace implementation.  x86-64 version.
   Copyright (C) 2024 Free Software Foundation, Inc.
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

#include <stdbool.h>
#include <sysdep.h>

/* From libc_sigaction.c.  */
extern void restore_rt (void) asm ("__restore_rt") attribute_hidden;

/* Copy addresses from the shadow stack if available.  */
static inline __always_inline int
__arch_backtrace (void **array, int size)
{
#if CET_ENABLED
  void **ssp;
  asm ("rdsspq %0"
       : "=r" (ssp)
       : "0" (0));
  if (ssp == NULL)
    return -1;

  void **ssp_base = (void **) THREAD_GETMEM (THREAD_SELF, header.ssp_base);
  if (ssp_base < ssp)
    /* Covers the NULL case.  */
    return 0;

  long int limit = ssp_base - ssp;
  if (limit > size)
    limit = size;

#if 1
  __builtin_memcpy (array, ssp, limit * sizeof (*array));
  return limit;
#else
  /* We cannot use memcpy because we need to filter out signal
     frames.  */
  int count = 0;
  for (unsigned int i = 0; i < limit; ++i)
    if (ssp[i] != restore_rt)
      array[count++] = ssp[i];
  return count;
#endif

#else /* !CET_ENABLED */
  return -1;
#endif
}
