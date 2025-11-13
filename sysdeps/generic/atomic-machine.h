/* Copyright (C) 2003-2025 Free Software Foundation, Inc.
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

#ifndef _ATOMIC_MACHINE_H
#define _ATOMIC_MACHINE_H	1

/* Some macros can be overridden if the architecture requires some specific
   atomic operations or provides extra optimizations.

   * atomic_max (mem, value): atomically set the maximum value of *mem
     and value to *mem.  Used on malloc statistics collection.

   * atomic_full_barrier: defaults to __atomic_thread_fence (__ATOMIC_SEQ_CST)

   * atomic_read_barrier: defaults to __atomic_thread_fence (__ATOMIC_ACQUIRE)

   * atomic_write_barrier: defaults to __atomic_thread_fence (__ATOMIC_RELEASE)

   * atomic_spin_nop: arch-specific instruction used on pthread spin lock
     and adaptive mutexes to optimize spin-wait loops.
*/

#include <bits/wordsize.h>

/* NB: The NPTL semaphore code casts a sem_t to a new_sem and issues a 64-bit
   atomic operation for USE_64B_ATOMICS.  However, the sem_t has 32-bit
   alignment on 32-bit architectures, which prevents using 64-bit atomics even
   if the ABI supports it.  */
#if __WORDSIZE == 64
# define USE_64B_ATOMICS 1
#else
# define USE_64B_ATOMICS 0
#endif

#endif	/* atomic-machine.h */
