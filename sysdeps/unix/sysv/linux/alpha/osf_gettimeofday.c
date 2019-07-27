/* gettimeofday -- Get the current time of day.  Linux/Alpha/tv32 version.
   Copyright (C) 2019 Free Software Foundation, Inc.
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
#include <limits.h>
#include <sys/time.h>
#include <sysdep.h>
#include <shlib-compat.h>

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)

struct timeval32
{
    int tv_sec, tv_usec;
};

/* Get the current time of day and timezone information.  */
int
attribute_compat_text_section
__gettimeofday_tv32 (struct timeval32 *tv32,
                     struct timezone *tz)
{
  struct timeval tv64;
  if (__gettimeofday (&tv64, tz))
    return -1;

  /* The tv_sec field of a 64-bit struct timeval will overflow the
     range representable by 'int' at 2038-01-19 03:14:07 +0000.  */
  if (tv64.tv_sec > (time_t)INT_MAX)
    {
      tv32.tv_sec = INT_MAX;
      tv32.tv_usec = 0;
      __set_errno (EOVERFLOW);
      return -1;
    }
  tv32.tv_sec = (int)tv64.tv_sec;

  /* The tv_usec field of a 64-bit struct timeval may be a 64-bit
     type, but it never contains a value outside the range [0, 999999],
     so this cast is guaranteed not to lose information.  */
  tv32.tv_usec = (int)tv64.tv_usec;

  return 0;
}

compat_symbol (libc, __gettimeofday_tv32, gettimeofday, GLIBC_2_0);
#endif
