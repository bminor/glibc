/* Copyright (C) 1999-2019 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <shlib-compat.h>

/* Set CLOCK to value TP.  */
int
__clock_settime (clockid_t clock_id, const struct timespec *tp)
{
  int retval = -1;

  /* Make sure the time cvalue is OK.  */
  if (tp->tv_nsec < 0 || tp->tv_nsec >= 1000000000)
    {
      __set_errno (EINVAL);
      return -1;
    }

  switch (clock_id)
    {
    case CLOCK_REALTIME:
      {
	struct timeval tv;
	TIMESPEC_TO_TIMEVAL (&tv, tp);
	retval = __settimeofday (&tv, NULL);
      }
      break;

    default:
      __set_errno (EINVAL);
      break;
    }

  return retval;
}
libc_hidden_def (__clock_settime)

versioned_symbol (libc, __clock_settime, clock_settime, GLIBC_2_17);
/* clock_settime moved to libc in version 2.17;
   old binaries may expect the symbol version it had in librt.  */
#if SHLIB_COMPAT (libc, GLIBC_2_2, GLIBC_2_17)
strong_alias (__clock_settime, __clock_settime_2);
compat_symbol (libc, __clock_settime_2, clock_settime, GLIBC_2_2);
#endif
