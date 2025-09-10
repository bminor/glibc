/* Low-level functions for atomic operations. Mips version.
   Copyright (C) 2005-2025 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _MIPS_ATOMIC_MACHINE_H
#define _MIPS_ATOMIC_MACHINE_H 1

#include <sgidefs.h>

#if _MIPS_SIM == _ABIO32 && __mips < 2
#define MIPS_PUSH_MIPS2 ".set	mips2\n\t"
#else
#define MIPS_PUSH_MIPS2
#endif

#if _MIPS_SIM == _ABIO32 || _MIPS_SIM == _ABIN32
#define __HAVE_64B_ATOMICS 0
#else
#define __HAVE_64B_ATOMICS 1
#endif

/* See the comments in <sys/asm.h> about the use of the sync instruction.  */
#ifndef MIPS_SYNC
# define MIPS_SYNC	sync
#endif

#define MIPS_SYNC_STR_2(X) #X
#define MIPS_SYNC_STR_1(X) MIPS_SYNC_STR_2(X)
#define MIPS_SYNC_STR MIPS_SYNC_STR_1(MIPS_SYNC)

/* MIPS is an LL/SC machine.  However, XLP has a direct atomic exchange
   instruction which will be used by __atomic_exchange_n.  */
#ifdef _MIPS_ARCH_XLP
# define ATOMIC_EXCHANGE_USES_CAS 0
#else
# define ATOMIC_EXCHANGE_USES_CAS 1
#endif

#ifdef __mips16
# define atomic_full_barrier() __sync_synchronize ()

#else /* !__mips16 */
# define atomic_full_barrier() \
  __asm__ __volatile__ (".set push\n\t"					      \
			MIPS_PUSH_MIPS2					      \
			MIPS_SYNC_STR "\n\t"				      \
			".set pop" : : : "memory")
#endif /* !__mips16 */

#endif /* atomic-machine.h */
