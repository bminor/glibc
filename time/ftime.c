/* Deprecated return date and time.
   Copyright (C) 1994-2020 Free Software Foundation, Inc.
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

#include <shlib-compat.h>

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_33)

#include <time.h>

struct timeb
  {
    time_t time;		/* Seconds since epoch, as from `time'.  */
    unsigned short int millitm;	/* Additional milliseconds.  */
    short int timezone;		/* Minutes west of GMT.  */
    short int dstflag;		/* Nonzero if Daylight Savings Time used.  */
  };

int
attribute_compat_text_section
__ftime (struct timeb *timebuf)
{
  struct timespec ts;
  __clock_gettime (CLOCK_REALTIME, &ts);

  timebuf->time = ts.tv_sec;
  timebuf->millitm = ts.tv_nsec / 1000000;
  timebuf->timezone = 0;
  timebuf->dstflag = 0;
  return 0;
}

compat_symbol (libc, __ftime, ftime, GLIBC_2_0);
#endif
