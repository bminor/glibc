/* Atomic operations.  X86 version.
   Copyright (C) 2018-2025 Free Software Foundation, Inc.
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

#ifndef _X86_ATOMIC_MACHINE_H
#define _X86_ATOMIC_MACHINE_H 1

#include <stdint.h>
#include <libc-pointer-arith.h>		/* For cast_to_integer.  */

#define USE_ATOMIC_COMPILER_BUILTINS	1

#ifdef __x86_64__
# define __HAVE_64B_ATOMICS		1
#else
/* Since the Pentium, i386 CPUs have supported 64-bit atomics, but the
   i386 psABI supplement provides only 4-byte alignment for uint64_t
   inside structs, so it is currently not possible to use 64-bit
   atomics on this platform.  */
# define __HAVE_64B_ATOMICS		0
#endif

#define ATOMIC_EXCHANGE_USES_CAS	0

#define atomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  __sync_val_compare_and_swap (mem, oldval, newval)
#define atomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
  (! __sync_bool_compare_and_swap (mem, oldval, newval))

/* Note that we need no lock prefix.  */
#define atomic_exchange_acq(mem, newvalue) \
  ({ __typeof (*mem) result;						      \
     if (sizeof (*mem) == 1)						      \
       __asm __volatile ("xchgb %b0, %1"				      \
			 : "=q" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "m" (*mem));			      \
     else if (sizeof (*mem) == 2)					      \
       __asm __volatile ("xchgw %w0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "m" (*mem));			      \
     else if (sizeof (*mem) == 4)					      \
       __asm __volatile ("xchgl %0, %1"					      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" (newvalue), "m" (*mem));			      \
     else if (__HAVE_64B_ATOMICS)					      \
       __asm __volatile ("xchgq %q0, %1"				      \
			 : "=r" (result), "=m" (*mem)			      \
			 : "0" ((int64_t) cast_to_integer (newvalue)),        \
			   "m" (*mem));					      \
     else								      \
       {								      \
	 result = 0;							      \
	 __atomic_link_error ();					      \
       }								      \
     result; })

/* ??? Remove when catomic_exchange_and_add
   fallback uses __atomic_fetch_add.  */
#define catomic_exchange_and_add(mem, value) \
  __atomic_fetch_add (mem, value, __ATOMIC_ACQUIRE)

#define atomic_full_barrier() __sync_synchronize ()
#define atomic_read_barrier() __asm ("" ::: "memory")
#define atomic_write_barrier() __asm ("" ::: "memory")

#define atomic_spin_nop() __asm ("pause")

#endif /* atomic-machine.h */
