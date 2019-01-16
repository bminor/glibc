/* clock_gettime -- Get the current time from a POSIX clockid_t.  Unix version.
   Copyright (C) 1999-2019 Free Software Foundation, Inc.
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
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <libc-internal.h>
#include <ldsodefs.h>


static inline int
realtime_gettime (struct timespec *tp)
{
  struct timeval tv;
  int retval = __gettimeofday (&tv, NULL);
  if (retval == 0)
    /* Convert into `timespec'.  */
    TIMEVAL_TO_TIMESPEC (&tv, tp);
  return retval;
}


/* Get current value of CLOCK and store it in TP.  */
int
__clock_gettime (clockid_t clock_id, struct timespec *tp)
{
  int retval = -1;

  switch (clock_id)
    {
    case CLOCK_REALTIME:
      {
	struct timeval tv;
	retval = __gettimeofday (&tv, NULL);
	if (retval == 0)
	  TIMEVAL_TO_TIMESPEC (&tv, tp);
      }
      break;

    default:
      __set_errno (EINVAL);
      break;
    }

  return retval;
}
weak_alias (__clock_gettime, clock_gettime)
libc_hidden_def (__clock_gettime)
