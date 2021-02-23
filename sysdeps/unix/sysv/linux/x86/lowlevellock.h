/* Low-level lock implementation, x86 version.
   Copyright (C) 2019-2021 Free Software Foundation, Inc.
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

#ifndef _X86_64_LOWLEVELLOCK_H
#define _X86_64_LOWLEVELLOCK_H	1

#ifndef __ASSEMBLER__
#include <sysdeps/nptl/lowlevellock.h>
#include <single-thread.h>

/* The lll_trylock, lll_lock, and lll_unlock implements a single-thread
   optimization using the cmpxchgl instruction.  It checks if the process
   is single thread and avoid a more expensive atomic instruction.  */

/* The single-thread optimization only works for libc itself, we need
   atomicity for libpthread in case of shared futexes.  */
#if !IS_IN(libc)
# define is_single_thread 0
#else
# define is_single_thread SINGLE_THREAD_P
#endif

/* In the __lllc_as we simply return the value in %eax after the cmpxchg
   instruction.  In case the operation succeeded this value is zero.  In case
   the operation failed, the cmpxchg instruction has loaded the current value
   of the memory work which is guaranteed to be nonzero.  */
static inline int
__attribute__ ((always_inline))
__lll_cas_lock (int *futex)
{
  int ret;
  asm volatile ("cmpxchgl %2, %1"
		: "=a" (ret), "=m" (*futex)
		: "r" (1), "m" (*futex), "0" (0)
		: "memory");
  return ret;
}

#undef lll_trylock
#define lll_trylock(lock)						     \
  ({									     \
    int __ret;								     \
    if (is_single_thread)						     \
      __ret = __lll_cas_lock (&(lock));					     \
    else								     \
      __ret = __lll_trylock (&(lock));					     \
    __ret;								     \
  })

#undef lll_lock
#define lll_lock(lock, private)						     \
  ((void)								     \
   ({									     \
     if (is_single_thread)						     \
       __lll_cas_lock (&(lock));					     \
     else								     \
       __lll_lock (&(lock), private);					     \
   }))

#undef lll_unlock
#define lll_unlock(lock, private)					     \
  ((void)								     \
   ({									     \
     if (is_single_thread)						     \
       (lock)--;							     \
     else								     \
       __lll_unlock (&(lock), private);					     \
   }))

#endif  /* !__ASSEMBLER__ */

#endif	/* lowlevellock.h */
