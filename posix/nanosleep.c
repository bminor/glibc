/* Copyright (C) 1996-2025 Free Software Foundation, Inc.
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

#include <errno.h>
#include <time.h>


/* Pause execution for a number of nanoseconds.  */
int
__nanosleep (const struct timespec *requested_time,
	     struct timespec *remaining)
{
  int ret = __clock_nanosleep (CLOCK_REALTIME, 0, requested_time, remaining);
  if (ret != 0)
    {
      __set_errno (ret);
      return -1;
    }
  return 0;
}
libc_hidden_def (__nanosleep)
weak_alias (__nanosleep, nanosleep)
