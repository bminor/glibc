/* Set the current time of day and timezone information.

   Copyright (C) 2018 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <errno.h>
#include <sys/time.h>

int __settimeofday64(const struct __timeval64 *tv,
                       const struct timezone *tz)
{
  struct timeval tv32;

  if (tv && tv->tv_sec > INT_MAX)
    {
      __set_errno(EOVERFLOW);
      return -1;
    }

  tv32.tv_sec = tv->tv_sec;
  tv32.tv_usec = tv->tv_usec;

  return __settimeofday(&tv32, tz);
}
