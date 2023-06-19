/* Copyright (C) 2003-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <nptl/descr.h>

extern __typeof (timer_create) __timer_create;
libc_hidden_proto (__timer_create)
extern __typeof (timer_delete) __timer_delete;
libc_hidden_proto (__timer_delete)
extern __typeof (timer_getoverrun) __timer_getoverrun;
libc_hidden_proto (__timer_getoverrun)

/* Type of timers in the kernel.  */
typedef int kernel_timer_t;

/* For !SIGEV_THREAD, the resulting 'timer_t' is the returned kernel timer
   identifier (kernel_timer_t), while for SIGEV_THREAD it assumes the
   pthread_t at least 8-bytes aligned.

   For SIGEV_THREAD, the MSB bit (INT_MAX) is used on timer_delete to
   signal the helper thread to stop and issue the timer_delete syscall.  */

static inline timer_t
kernel_timer_to_timerid (kernel_timer_t ktimerid)
{
  return (timer_t) ((intptr_t) ktimerid);
}

static inline timer_t
pthread_to_timerid (pthread_t ptr)
{
  return (timer_t) (INTPTR_MIN | (uintptr_t) ptr >> 1);
}

static inline bool
timer_is_sigev_thread (timer_t timerid)
{
  return (intptr_t) timerid < 0;
}

static inline struct pthread *
timerid_to_pthread (timer_t timerid)
{
  return (struct pthread *)((uintptr_t) timerid << 1);
}

static inline kernel_timer_t
timerid_to_kernel_timer (timer_t timerid)
{
  if (timer_is_sigev_thread (timerid))
    {
      struct pthread *pthr = timerid_to_pthread (timerid);
      return pthr->timerid & INT_MAX;
    }
  return (uintptr_t) timerid;
}

static inline void
timerid_signal_delete (kernel_timer_t *timerid)
{
  atomic_fetch_or_relaxed (timerid, INT_MIN);
}

static inline kernel_timer_t
timerid_clear (kernel_timer_t timerid)
{
  return timerid & INT_MAX;
}

/* New targets use int instead of timer_t.  The difference only
   matters on 64-bit targets.  */
#include <timer_t_was_int_compat.h>

#if TIMER_T_WAS_INT_COMPAT
# define OLD_TIMER_MAX 256
extern timer_t __timer_compat_list[OLD_TIMER_MAX];
#endif
