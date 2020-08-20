/* Auxiliary definitions for 64-bit time_t support.
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

#include <stdbool.h>
#include <atomic.h>

/* These helper functions are used to optimize the 64-bit time_t support on
   configurations that requires support for 32-bit time_t fallback
   (!__ASSUME_TIME64_SYSCALLS).  The idea is once the kernel advertises that
   it does not have 64-bit time_t support, glibc will stop to try issue the
   64-bit time_t syscall altogether.

   For instance:

     #ifndef __NR_symbol_time64
     # define __NR_symbol_time64 __NR_symbol
     #endif
     int r;
     if (supports_time64 ())
       {
         r = INLINE_SYSCALL_CALL (symbol, ...);
         if (r == 0 || errno != ENOSYS)
	   return r;

         mark_time64_unsupported ();
       }
     #ifndef __ASSUME_TIME64_SYSCALLS
     <32-bit fallback syscall>
     #endif
     return r;

   On configuration with default 64-bit time_t this optimization should be
   optimized away by the compiler resulting in no overhead.  */

#ifndef __ASSUME_TIME64_SYSCALLS
extern int __time64_support attribute_hidden;
#endif

static inline bool
supports_time64 (void)
{
#ifdef __ASSUME_TIME64_SYSCALLS
  return true;
#else
  return atomic_load_relaxed (&__time64_support) != 0;
#endif
}

static inline void
mark_time64_unsupported (void)
{
#ifndef __ASSUME_TIME64_SYSCALLS
  atomic_store_relaxed (&__time64_support, 0);
#endif
}
