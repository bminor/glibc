/* Pause execution for a number of nanoseconds

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

#include <errno.h>
#include <time.h>

/* Pause execution for a number of nanoseconds.  */
int
__nanosleep64 (const struct __timespec64 *requested_time,
	         struct __timespec64 *remaining)
{
  struct timespec treq32, *treqp32 = NULL;
  struct timespec trem32, *tremp32 = NULL;

  if (requested_time)
    {
      if (requested_time->tv_sec > INT_MAX)
        {
          __set_errno(EOVERFLOW);
          return -1;
        }
      treq32.tv_sec = requested_time->tv_sec;
      treq32.tv_nsec = requested_time->tv_nsec;
      treqp32 = & treq32;
    }

  if (remaining)
    tremp32 = &trem32;

  int result = __nanosleep(treqp32, tremp32);

  if (result == 1 && errno == EINTR && remaining)
    {
      remaining->tv_sec = trem32.tv_sec;
      remaining->tv_nsec = trem32.tv_nsec;
      remaining->tv_pad = 0;
    }

  return result;
}
