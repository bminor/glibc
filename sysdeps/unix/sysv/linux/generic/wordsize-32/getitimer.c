/* getitimer -- Get the state of an interval timer.  Linux/tv32 version.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#include <sys/time.h>
#include <sysdep.h>
#include <tv32-compat.h>

int
__getitimer (__itimer_which_t which, struct itimerval *curr_value)
{
#if __TIMESIZE == 64
  struct itimerval32 curr_value_32;
  if (INLINE_SYSCALL_CALL (getitimer, which, &curr_value_32) == -1)
    return -1;

  /* Write all fields of 'curr_value' regardless of overflow.  */
  curr_value->it_interval
    = valid_timeval_long_to_timeval64 (curr_value_32.it_interval);
  curr_value->it_value
    = valid_timeval_long_to_timeval64 (curr_value_32.it_value);
  return 0;
#else
  return INLINE_SYSCALL_CALL (getitimer, which, curr_value);
#endif
}

weak_alias (__getitimer, getitimer)
