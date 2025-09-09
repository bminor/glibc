/* Atomic operations.  PowerPC32 version.
   Copyright (C) 2003-2025 Free Software Foundation, Inc.
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

/*  POWER6 adds a "Mutex Hint" to the Load and Reserve instruction.
    This is a hint to the hardware to expect additional updates adjacent
    to the lock word or not.  If we are acquiring a Mutex, the hint
    should be true. Otherwise we releasing a Mutex or doing a simple
    atomic operation.  In that case we don't expect additional updates
    adjacent to the lock word after the Store Conditional and the hint
    should be false.  */

#if defined _ARCH_PWR6 || defined _ARCH_PWR6X
# define MUTEX_HINT_ACQ	",1"
# define MUTEX_HINT_REL	",0"
#else
# define MUTEX_HINT_ACQ
# define MUTEX_HINT_REL
#endif

#define __HAVE_64B_ATOMICS 0
#define USE_ATOMIC_COMPILER_BUILTINS 1
#define ATOMIC_EXCHANGE_USES_CAS 1

#ifdef _ARCH_PWR4
/*
 * Newer powerpc64 processors support the new "light weight" sync (lwsync)
 * So if the build is using -mcpu=[power4,power5,power5+,970] we can
 * safely use lwsync.
 */
# define atomic_read_barrier()	__asm ("lwsync" ::: "memory")
/*
 * "light weight" sync can also be used for the release barrier.
 */
# define __ARCH_REL_INSTR	"lwsync"
# define atomic_write_barrier()	__asm ("lwsync" ::: "memory")
#else
/*
 * Older powerpc32 processors don't support the new "light weight"
 * sync (lwsync).  So the only safe option is to use normal sync
 * for all powerpc32 applications.
 */
# define atomic_read_barrier()	__asm ("sync" ::: "memory")
# define atomic_write_barrier()	__asm ("sync" ::: "memory")
#endif

/*
 * Include the rest of the atomic ops macros which are common to both
 * powerpc32 and powerpc64.
 */
#include_next <atomic-machine.h>
