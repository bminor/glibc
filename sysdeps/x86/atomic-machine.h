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
#include <tls.h>			/* For tcbhead_t.  */
#include <libc-pointer-arith.h>		/* For cast_to_integer.  */

#define LOCK_PREFIX "lock;"

#define USE_ATOMIC_COMPILER_BUILTINS	1

#ifdef __x86_64__
# define __HAVE_64B_ATOMICS		1
# define SP_REG				"rsp"
# define SEG_REG			"fs"
# define BR_CONSTRAINT			"q"
#else
/* Since the Pentium, i386 CPUs have supported 64-bit atomics, but the
   i386 psABI supplement provides only 4-byte alignment for uint64_t
   inside structs, so it is currently not possible to use 64-bit
   atomics on this platform.  */
# define __HAVE_64B_ATOMICS		0
# define SP_REG				"esp"
# define SEG_REG			"gs"
# define BR_CONSTRAINT			"r"
#endif
#define ATOMIC_EXCHANGE_USES_CAS	0

#define atomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  __sync_val_compare_and_swap (mem, oldval, newval)
#define atomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
  (! __sync_bool_compare_and_swap (mem, oldval, newval))


#define __arch_c_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile ("cmpl $0, %%" SEG_REG ":%P5\n\t"			      \
		       "je 0f\n\t"					      \
		       "lock\n"						      \
		       "0:\tcmpxchgb %b2, %1"				      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : BR_CONSTRAINT (newval), "m" (*mem), "0" (oldval),    \
			 "i" (offsetof (tcbhead_t, multiple_threads)));	      \
     ret; })

#define __arch_c_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile ("cmpl $0, %%" SEG_REG ":%P5\n\t"			      \
		       "je 0f\n\t"					      \
		       "lock\n"						      \
		       "0:\tcmpxchgw %w2, %1"				      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : BR_CONSTRAINT (newval), "m" (*mem), "0" (oldval),    \
			 "i" (offsetof (tcbhead_t, multiple_threads)));	      \
     ret; })

#define __arch_c_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile ("cmpl $0, %%" SEG_REG ":%P5\n\t"			      \
		       "je 0f\n\t"					      \
		       "lock\n"						      \
		       "0:\tcmpxchgl %2, %1"				      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : BR_CONSTRAINT (newval), "m" (*mem), "0" (oldval),    \
			 "i" (offsetof (tcbhead_t, multiple_threads)));       \
     ret; })

#ifdef __x86_64__
# define __arch_c_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret;						      \
     __asm __volatile ("cmpl $0, %%fs:%P5\n\t"				      \
		       "je 0f\n\t"					      \
		       "lock\n"						      \
		       "0:\tcmpxchgq %q2, %1"				      \
		       : "=a" (ret), "=m" (*mem)			      \
		       : "q" ((int64_t) cast_to_integer (newval)),	      \
			 "m" (*mem),					      \
			 "0" ((int64_t) cast_to_integer (oldval)),	      \
			 "i" (offsetof (tcbhead_t, multiple_threads)));	      \
     ret; })
# define do_add_val_64_acq(pfx, mem, value) do { } while (0)
#else
# define __arch_c_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __typeof (*mem) ret = *(mem);					      \
     __atomic_link_error ();						      \
     ret = (newval);							      \
     ret = (oldval);							      \
     ret; })

# define do_add_val_64_acq(pfx, mem, value) \
  {									      \
    __typeof (value) __addval = (value);				      \
    __typeof (mem) __memp = (mem);					      \
    __typeof (*mem) __oldval = *__memp;					      \
    __typeof (*mem) __tmpval;						      \
    do									      \
      __tmpval = __oldval;						      \
    while ((__oldval = pfx##_compare_and_exchange_val_64_acq		      \
	    (__memp, __oldval + __addval, __oldval)) == __tmpval);	      \
  }
#endif


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

#define __arch_decrement_body(lock, pfx, mem) \
  do {									      \
    if (sizeof (*mem) == 1)						      \
      __asm __volatile (lock "decb %b0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 2)					      \
      __asm __volatile (lock "decw %w0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (sizeof (*mem) == 4)					      \
      __asm __volatile (lock "decl %0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else if (__HAVE_64B_ATOMICS)					      \
      __asm __volatile (lock "decq %q0"					      \
			: "=m" (*mem)					      \
			: "m" (*mem),					      \
			  "i" (offsetof (tcbhead_t, multiple_threads)));      \
    else								      \
      do_add_val_64_acq (pfx, mem, -1);					      \
  } while (0)

#define __arch_decrement_cprefix \
  "cmpl $0, %%" SEG_REG ":%P2\n\tje 0f\n\tlock\n0:\t"

#define catomic_decrement(mem) \
  __arch_decrement_body (__arch_decrement_cprefix, __arch_c, mem)

/* We don't use mfence because it is supposedly slower due to having to
   provide stronger guarantees (e.g., regarding self-modifying code).  */
#define atomic_full_barrier() \
    __asm __volatile (LOCK_PREFIX "orl $0, (%%" SP_REG ")" ::: "memory")
#define atomic_read_barrier() __asm ("" ::: "memory")
#define atomic_write_barrier() __asm ("" ::: "memory")

#define atomic_spin_nop() __asm ("pause")

#endif /* atomic-machine.h */
