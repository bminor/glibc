/* Copyright (C) 2003-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LOWLEVELLOCK_H
#define _LOWLEVELLOCK_H	1

#include <time.h>
#include <sys/param.h>
#include <bits/pthreadtypes.h>
#include <atomic.h>
#include <kernel-features.h>

#include <lowlevellock-futex.h>

static inline int
__attribute__ ((always_inline))
__lll_trylock (int *futex)
{
  return atomic_compare_and_exchange_val_24_acq (futex, 1, 0) != 0;
}
#define lll_trylock(futex) __lll_trylock (&(futex))

static inline int
__attribute__ ((always_inline))
__lll_cond_trylock (int *futex)
{
  return atomic_compare_and_exchange_val_24_acq (futex, 2, 0) != 0;
}
#define lll_cond_trylock(futex) __lll_cond_trylock (&(futex))


extern void __lll_lock_wait_private (int *futex) attribute_hidden;
extern void __lll_lock_wait (int *futex, int private) attribute_hidden;

static inline void
__attribute__ ((always_inline))
__lll_lock (int *futex, int private)
{
  int val = atomic_compare_and_exchange_val_24_acq (futex, 1, 0);

  if (__glibc_unlikely (val != 0))
    {
      if (__builtin_constant_p (private) && private == LLL_PRIVATE)
	__lll_lock_wait_private (futex);
      else
	__lll_lock_wait (futex, private);
    }
}
#define lll_lock(futex, private) __lll_lock (&(futex), private)

static inline void
__attribute__ ((always_inline))
__lll_cond_lock (int *futex, int private)
{
  int val = atomic_compare_and_exchange_val_24_acq (futex, 2, 0);

  if (__glibc_unlikely (val != 0))
    __lll_lock_wait (futex, private);
}
#define lll_cond_lock(futex, private) __lll_cond_lock (&(futex), private)


extern int __lll_timedlock_wait (int *futex, const struct timespec *,
				 int private) attribute_hidden;

static inline int
__attribute__ ((always_inline))
__lll_timedlock (int *futex, const struct timespec *abstime, int private)
{
  int val = atomic_compare_and_exchange_val_24_acq (futex, 1, 0);
  int result = 0;

  if (__glibc_unlikely (val != 0))
    result = __lll_timedlock_wait (futex, abstime, private);
  return result;
}
#define lll_timedlock(futex, abstime, private) \
  __lll_timedlock (&(futex), abstime, private)

#define lll_unlock(lock, private) \
  ((void) ({								      \
    int *__futex = &(lock);						      \
    int __private = (private);						      \
    int __val = atomic_exchange_24_rel (__futex, 0);			      \
    if (__glibc_unlikely (__val > 1))					      \
      lll_futex_wake (__futex, 1, __private);				      \
  }))

#define lll_islocked(futex) \
  (futex != 0)

/* Initializers for lock.  */
#define LLL_LOCK_INITIALIZER		(0)
#define LLL_LOCK_INITIALIZER_LOCKED	(1)

extern int __lll_timedwait_tid (int *, const struct timespec *)
     attribute_hidden;

/* The kernel notifies a process which uses CLONE_CHILD_CLEARTID via futex
   wake-up when the clone terminates.  The memory location contains the
   thread ID while the clone is running and is reset to zero by the kernel
   afterwards.  The kernel up to version 3.16.3 does not use the private futex
   operations for futex wake-up when the clone terminates.
   If ABSTIME is not NULL, is used a timeout for futex call.  If the timeout
   occurs then return ETIMEOUT, if ABSTIME is invalid, return EINVAL.
   The futex operation are issues with cancellable versions.  */
#define lll_wait_tid(tid, abstime)					\
  ({									\
    int __res = 0;							\
    __typeof (tid) __tid;						\
    if (abstime != NULL)						\
      __res = __lll_timedwait_tid (&(tid), (abstime));			\
    else								\
      /* We need acquire MO here so that we synchronize with the 	\
	 kernel's store to 0 when the clone terminates. (see above)  */	\
      while ((__tid = atomic_load_acquire (&(tid))) != 0)		\
        lll_futex_wait_cancel (&(tid), __tid, LLL_SHARED);		\
    __res;								\
  })

#endif	/* lowlevellock.h */
