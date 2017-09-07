/* clock_getres -- Get the resolution of a POSIX clockid_t.
   Copyright (C) 1999-2018 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <sys/param.h>
#include <libc-internal.h>

#if HP_TIMING_AVAIL
static long int nsec;		/* Clock frequency of the processor.  */

static int
hp_timing_getres (struct __timespec64 *res)
{
  if (__glibc_unlikely (nsec == 0))
    {
      hp_timing_t freq;

      /* This can only happen if we haven't initialized the `nsec'
	 variable yet.  Do this now.  We don't have to protect this
	 code against multiple execution since all of them should
	 lead to the same result.  */
      freq = __get_clockfreq ();
      if (__glibc_unlikely (freq == 0))
	/* Something went wrong.  */
	return -1;

      nsec = MAX (UINT64_C (1000000000) / freq, 1);
    }

  /* Fill in the values.
     The seconds are always zero (unless we have a 1Hz machine).  */
  res->tv_sec = 0;
  res->tv_nsec = nsec;

  return 0;
}
#endif

static inline int
realtime_getres (struct __timespec64 *res)
{
  long int clk_tck = __sysconf (_SC_CLK_TCK);

  if (__glibc_likely (clk_tck != -1))
    {
      /* This implementation assumes that the realtime clock has a
	 resolution higher than 1 second.  This is the case for any
	 reasonable implementation.  */
      res->tv_sec = 0;
      res->tv_nsec = 1000000000 / clk_tck;
      return 0;
    }

  return -1;
}

/* Get resolution of clock.  */
int
__clock_getres_time64 (clockid_t clock_id, struct __timespec64 *res)
{
  int retval = -1;

  switch (clock_id)
    {
#ifdef SYSDEP_GETRES64
      SYSDEP_GETRES64;
#endif

#ifndef HANDLED_REALTIME
    case CLOCK_REALTIME:
      retval = realtime_getres (res);
      break;
#endif	/* handled REALTIME */

    default:
#ifdef SYSDEP_GETRES_CPU64
      SYSDEP_GETRES_CPU64;
#endif
#if HP_TIMING_AVAIL
      if ((clock_id & ((1 << CLOCK_IDFIELD_SIZE) - 1))
	  == CLOCK_THREAD_CPUTIME_ID)
	retval = hp_timing_getres (res);
      else
#endif
	__set_errno (EINVAL);
      break;

#if HP_TIMING_AVAIL && !defined HANDLED_CPUTIME
    case CLOCK_PROCESS_CPUTIME_ID:
    case CLOCK_THREAD_CPUTIME_ID:
      retval = hp_timing_getres (res);
      break;
#endif
    }

  return retval;
}

int
__clock_getres (clockid_t clock_id, struct timespec *res)
{
  struct __timespec64 ts64;
  int retval = __clock_getres_time64 (clock_id, &ts64);
  if (retval == 0)
    {
      // We assume we never run with a CPU clock period greater than
      // 2**31 seconds and therefore we do not check the seconds field
      res->tv_sec = ts64.tv_sec;
      res->tv_nsec = ts64.tv_nsec;
    }
  return retval;
}
weak_alias (__clock_getres, clock_getres)
