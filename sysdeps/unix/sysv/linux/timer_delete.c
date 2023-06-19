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

#include <unistd.h>
#include <time.h>
#include "kernel-posix-timers.h"
#include <pthreadP.h>
#include <shlib-compat.h>

int
___timer_delete (timer_t timerid)
{
  if (timer_is_sigev_thread (timerid))
    {
      struct pthread *th = timerid_to_pthread (timerid);

      /* The helper thread itself will be responsible to call the
	 timer_delete syscall.  */
      timerid_signal_delete (&th->timerid);
      /* We can send the signal directly instead of through
	 __pthread_kill_internal because the thread is not user-visible
	 and it blocks SIGTIMER.  */
      INTERNAL_SYSCALL_CALL (tgkill, __getpid (), th->tid, SIGTIMER);
      return 0;
    }
  return INLINE_SYSCALL_CALL (timer_delete, timerid);
}
versioned_symbol (libc, ___timer_delete, timer_delete, GLIBC_2_34);
libc_hidden_ver (___timer_delete, __timer_delete)

#if TIMER_T_WAS_INT_COMPAT
# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_3_3, GLIBC_2_34)
compat_symbol (librt, ___timer_delete, timer_delete, GLIBC_2_3_3);
#endif

# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_3_3)
int
__timer_delete_old (int timerid)
{
  int res = __timer_delete (__timer_compat_list[timerid]);

  if (res == 0)
    /* Successful timer deletion, now free the index.  We only need to
       store a word and that better be atomic.  */
    __timer_compat_list[timerid] = NULL;

  return res;
}
compat_symbol (librt, __timer_delete_old, timer_delete, GLIBC_2_2);
# endif /* OTHER_SHLIB_COMPAT */

#else /* !TIMER_T_WAS_INT_COMPAT */
/* The transition from int to timer_t did not change ABI because the
   type sizes are the same.  */
# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_34)
compat_symbol (librt, ___timer_delete, timer_delete, GLIBC_2_2);
# endif
#endif /* !TIMER_T_WAS_INT_COMPAT */
