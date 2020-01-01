/* sem_clockwait -- wait on a semaphore with timeout using the specified
   clock.

   Copyright (C) 2019-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
   <https://www.gnu.org/licenses/>.  */

#include <time.h>
#include "sem_waitcommon.c"

int
sem_clockwait (sem_t *sem, clockid_t clockid,
	       const struct timespec *abstime)
{
  /* Check that supplied clockid is one we support, even if we don't end up
     waiting.  */
  if (!futex_abstimed_supported_clockid (clockid))
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (! valid_nanoseconds (abstime->tv_nsec))
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (__new_sem_wait_fast ((struct new_sem *) sem, 0) == 0)
    return 0;
  else
    return __new_sem_wait_slow ((struct new_sem *) sem, clockid, abstime);
}
