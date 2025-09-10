/* Internal macros for atomic operations for GNU C Library.
   Copyright (C) 2002-2025 Free Software Foundation, Inc.
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

#ifndef _ATOMIC_H
#define _ATOMIC_H	1

/* This header defines two types of macros:

   - atomic arithmetic and logic operation on memory.  They all
     have the prefix "atomic_".

   - support functions like barriers.  They also have the prefix
     "atomic_".

   Architectures must provide a few lowlevel macros (the compare
   and exchange definitions).  All others are optional.  They
   should only be provided if the architecture has specific
   support for the operation.

   As <atomic.h> macros are usually heavily nested and often use local
   variables to make sure side-effects are evaluated properly, use for
   macro local variables a per-macro unique prefix.  This file uses
   __atgN_ prefix where N is different in each macro.  */

#include <stdlib.h>

#include <atomic-machine.h>


# define atomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  ({									      \
     __typeof (*(mem)) __atg3_old = (oldval);				      \
     atomic_compare_exchange_acquire (mem, (void*)&__atg3_old, newval);	      \
     __atg3_old;							      \
  })

# define atomic_compare_and_exchange_val_rel(mem, newval, oldval)	      \
  ({									      \
     __typeof (*(mem)) __atg3_old = (oldval);				      \
     atomic_compare_exchange_release (mem, (void*)&__atg3_old, newval);	      \
     __atg3_old;							      \
  })

# define atomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
  ({									      \
     __typeof (*(mem)) __atg3_old = (oldval);				      \
     !atomic_compare_exchange_acquire (mem, (void*)&__atg3_old, newval);      \
  })

#ifndef atomic_max
# define atomic_max(mem, value) \
  do {									      \
    __typeof (*(mem)) __atg8_oldval;					      \
    __typeof (mem) __atg8_memp = (mem);					      \
    __typeof (*(mem)) __atg8_value = (value);				      \
    do {								      \
      __atg8_oldval = *__atg8_memp;					      \
      if (__atg8_oldval >= __atg8_value)				      \
	break;								      \
    } while (__builtin_expect						      \
	     (atomic_compare_and_exchange_bool_acq (__atg8_memp, __atg8_value,\
						    __atg8_oldval), 0));      \
  } while (0)
#endif


/* Decrement *MEM if it is > 0, and return the old value.  */
#ifndef atomic_decrement_if_positive
# define atomic_decrement_if_positive(mem) \
  ({ __typeof (*(mem)) __atg11_oldval;					      \
     __typeof (mem) __atg11_memp = (mem);				      \
									      \
     do									      \
       {								      \
	 __atg11_oldval = *__atg11_memp;				      \
	 if (__glibc_unlikely (__atg11_oldval <= 0))			      \
	   break;							      \
       }								      \
     while (__builtin_expect						      \
	    (atomic_compare_and_exchange_bool_acq (__atg11_memp,	      \
						   __atg11_oldval - 1,	      \
						   __atg11_oldval), 0));      \
     __atg11_oldval; })
#endif


#ifndef atomic_forced_read
# define atomic_forced_read(x) \
  ({ __typeof (x) __x; __asm ("" : "=r" (__x) : "0" (x)); __x; })
#endif

/* This is equal to 1 iff the architecture supports 64b atomic operations.  */
#ifndef __HAVE_64B_ATOMICS
#error Unable to determine if 64-bit atomics are present.
#endif

/* The following functions are a subset of the atomic operations provided by
   C11.  Usually, a function named atomic_OP_MO(args) is equivalent to C11's
   atomic_OP_explicit(args, memory_order_MO); exceptions noted below.  */

/* We require 32b atomic operations; some archs also support 64b atomic
   operations.  */
void __atomic_link_error (void);
# if __HAVE_64B_ATOMICS == 1
#  define __atomic_check_size(mem) \
   if ((sizeof (*mem) != 4) && (sizeof (*mem) != 8))			      \
     __atomic_link_error ();
# else
#  define __atomic_check_size(mem) \
   if (sizeof (*mem) != 4)						      \
     __atomic_link_error ();
# endif
/* We additionally provide 8b and 16b atomic loads and stores; we do not yet
   need other atomic operations of such sizes, and restricting the support to
   loads and stores makes this easier for archs that do not have native
   support for atomic operations to less-than-word-sized data.  */
# if __HAVE_64B_ATOMICS == 1
#  define __atomic_check_size_ls(mem) \
   if ((sizeof (*mem) != 1) && (sizeof (*mem) != 2) && (sizeof (*mem) != 4)   \
       && (sizeof (*mem) != 8))						      \
     __atomic_link_error ();
# else
#  define __atomic_check_size_ls(mem) \
   if ((sizeof (*mem) != 1) && (sizeof (*mem) != 2) && sizeof (*mem) != 4)    \
     __atomic_link_error ();
# endif

# define atomic_thread_fence_acquire() \
  __atomic_thread_fence (__ATOMIC_ACQUIRE)
# define atomic_thread_fence_release() \
  __atomic_thread_fence (__ATOMIC_RELEASE)
# define atomic_thread_fence_seq_cst() \
  __atomic_thread_fence (__ATOMIC_SEQ_CST)

# define atomic_load_relaxed(mem) \
  ({ __atomic_check_size_ls((mem));					      \
     __atomic_load_n ((mem), __ATOMIC_RELAXED); })
# define atomic_load_acquire(mem) \
  ({ __atomic_check_size_ls((mem));					      \
     __atomic_load_n ((mem), __ATOMIC_ACQUIRE); })

# define atomic_store_relaxed(mem, val) \
  do {									      \
    __atomic_check_size_ls((mem));					      \
    __atomic_store_n ((mem), (val), __ATOMIC_RELAXED);			      \
  } while (0)
# define atomic_store_release(mem, val) \
  do {									      \
    __atomic_check_size_ls((mem));					      \
    __atomic_store_n ((mem), (val), __ATOMIC_RELEASE);			      \
  } while (0)

/* On failure, this CAS has memory_order_relaxed semantics.  */
# define atomic_compare_exchange_weak_relaxed(mem, expected, desired) \
  ({ __atomic_check_size((mem));					      \
  __atomic_compare_exchange_n ((mem), (expected), (desired), 1,		      \
    __ATOMIC_RELAXED, __ATOMIC_RELAXED); })
# define atomic_compare_exchange_weak_acquire(mem, expected, desired) \
  ({ __atomic_check_size((mem));					      \
  __atomic_compare_exchange_n ((mem), (expected), (desired), 1,		      \
    __ATOMIC_ACQUIRE, __ATOMIC_RELAXED); })
# define atomic_compare_exchange_weak_release(mem, expected, desired) \
  ({ __atomic_check_size((mem));					      \
  __atomic_compare_exchange_n ((mem), (expected), (desired), 1,		      \
    __ATOMIC_RELEASE, __ATOMIC_RELAXED); })

# define atomic_compare_exchange_relaxed(mem, expected, desired) \
  ({ __atomic_check_size((mem));					      \
  __atomic_compare_exchange_n ((mem), (expected), (desired), 0,		      \
    __ATOMIC_RELAXED, __ATOMIC_RELAXED); })
# define atomic_compare_exchange_acquire(mem, expected, desired) \
  ({ __atomic_check_size((mem));					      \
  __atomic_compare_exchange_n ((mem), (expected), (desired), 0,		      \
    __ATOMIC_ACQUIRE, __ATOMIC_RELAXED); })
# define atomic_compare_exchange_release(mem, expected, desired) \
  ({ __atomic_check_size((mem));					      \
  __atomic_compare_exchange_n ((mem), (expected), (desired), 0,		      \
    __ATOMIC_RELEASE, __ATOMIC_RELAXED); })

# define atomic_exchange_relaxed(mem, desired) \
  ({ __atomic_check_size((mem));					      \
  __atomic_exchange_n ((mem), (desired), __ATOMIC_RELAXED); })
# define atomic_exchange_acquire(mem, desired) \
  ({ __atomic_check_size((mem));					      \
  __atomic_exchange_n ((mem), (desired), __ATOMIC_ACQUIRE); })
# define atomic_exchange_release(mem, desired) \
  ({ __atomic_check_size((mem));					      \
  __atomic_exchange_n ((mem), (desired), __ATOMIC_RELEASE); })

# define atomic_fetch_add_relaxed(mem, operand) \
  ({ __atomic_check_size((mem));					      \
  __atomic_fetch_add ((mem), (operand), __ATOMIC_RELAXED); })
# define atomic_fetch_add_acquire(mem, operand) \
  ({ __atomic_check_size((mem));					      \
  __atomic_fetch_add ((mem), (operand), __ATOMIC_ACQUIRE); })
# define atomic_fetch_add_release(mem, operand) \
  ({ __atomic_check_size((mem));					      \
  __atomic_fetch_add ((mem), (operand), __ATOMIC_RELEASE); })
# define atomic_fetch_add_acq_rel(mem, operand) \
  ({ __atomic_check_size((mem));					      \
  __atomic_fetch_add ((mem), (operand), __ATOMIC_ACQ_REL); })

# define atomic_fetch_and_relaxed(mem, operand) \
  ({ __atomic_check_size((mem));					      \
  __atomic_fetch_and ((mem), (operand), __ATOMIC_RELAXED); })
# define atomic_fetch_and_acquire(mem, operand) \
  ({ __atomic_check_size((mem));					      \
  __atomic_fetch_and ((mem), (operand), __ATOMIC_ACQUIRE); })
# define atomic_fetch_and_release(mem, operand) \
  ({ __atomic_check_size((mem));					      \
  __atomic_fetch_and ((mem), (operand), __ATOMIC_RELEASE); })

# define atomic_fetch_or_relaxed(mem, operand) \
  ({ __atomic_check_size((mem));					      \
  __atomic_fetch_or ((mem), (operand), __ATOMIC_RELAXED); })
# define atomic_fetch_or_acquire(mem, operand) \
  ({ __atomic_check_size((mem));					      \
  __atomic_fetch_or ((mem), (operand), __ATOMIC_ACQUIRE); })
# define atomic_fetch_or_release(mem, operand) \
  ({ __atomic_check_size((mem));					      \
  __atomic_fetch_or ((mem), (operand), __ATOMIC_RELEASE); })

# define atomic_fetch_xor_release(mem, operand) \
  ({ __atomic_check_size((mem));					      \
  __atomic_fetch_xor ((mem), (operand), __ATOMIC_RELEASE); })

/* This operation does not affect synchronization semantics but can be used
   in the body of a spin loop to potentially improve its efficiency.  */
#ifndef atomic_spin_nop
# define atomic_spin_nop() do { /* nothing */ } while (0)
#endif

#ifndef atomic_full_barrier
# define atomic_full_barrier() atomic_thread_fence_seq_cst ()
#endif

#ifndef atomic_read_barrier
# define atomic_read_barrier() atomic_thread_fence_acquire ()
#endif

#ifndef atomic_write_barrier
# define atomic_write_barrier() atomic_thread_fence_release ()
#endif


/* ATOMIC_EXCHANGE_USES_CAS is non-zero if atomic_exchange operations
   are implemented based on a CAS loop; otherwise, this is zero and we assume
   that the atomic_exchange operations could provide better performance
   than a CAS loop.  */
#ifndef ATOMIC_EXCHANGE_USES_CAS
# error ATOMIC_EXCHANGE_USES_CAS has to be defined.
#endif

#endif	/* atomic.h */
