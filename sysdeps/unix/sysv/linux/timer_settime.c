/* Copyright (C) 2003-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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
   not, see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sysdep.h>
#include <y2038-support.h>
#include "kernel-posix-timers.h"


#ifdef timer_settime_alias
# define timer_settime timer_settime_alias
#endif


int
timer_settime (timer_t timerid, int flags, const struct itimerspec *value,
	       struct itimerspec *ovalue)
{
#undef timer_settime
  struct timer *kt = (struct timer *) timerid;

  /* Delete the kernel timer object.  */
  int res = INLINE_SYSCALL (timer_settime, 4, kt->ktimerid, flags,
			    value, ovalue);

  return res;
}

/* 64-bit time version */

int
__timer_settime64 (timer_t timerid, int flags, const struct itimerspec *value,
                   struct itimerspec *ovalue)
{
  int res;
  struct timer *kt = (struct timer *) timerid;
  struct itimerspec value32, ovalue32;

  if (value == NULL)
  {
    __set_errno(EFAULT);
    return -1;
  }

#ifdef __NR_timer_settime64
  if (__y2038_get_kernel_support () > 0)
    {
      res = INLINE_SYSCALL (timer_settime, 3, kt->ktimerid, value, ovalue);
      if (res == 0 || errno != ENOSYS)
        return res;
      __y2038_set_kernel_support (-1);
    }
#endif

  if (value->it_value.tv_sec > INT_MAX
      || value->it_interval.tv_sec > INT_MAX)
    return EOVERFLOW;

  value32.it_value.tv_sec = value->it_value.tv_sec;
  value32.it_value.tv_nsec = value->it_value.tv_nsec;
  value32.it_interval.tv_sec = value->it_interval.tv_sec;
  value32.it_interval.tv_nsec = value->it_interval.tv_nsec;

  res = INLINE_SYSCALL (timer_settime, 4, kt->ktimerid, flags,
        &value32, &ovalue32);

  if (res == 0 && ovalue != NULL)
    {
      ovalue->it_value.tv_sec = ovalue32.it_value.tv_sec;
      ovalue->it_value.tv_nsec = ovalue32.it_value.tv_nsec;
      ovalue->it_interval.tv_sec = ovalue32.it_interval.tv_sec;
      ovalue->it_interval.tv_nsec = ovalue32.it_interval.tv_nsec;
    }

  return res;
}
