/* setitimer -- Set the state of an interval timer.  Linux/tv32 version.
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
__setitimer (__itimer_which_t which,
             const struct itimerval *restrict new_value,
             struct itimerval *restrict old_value)
{
#if __TIMESIZE == 64
  struct itimerval32 new_value_32;
  new_value_32.it_interval
    = valid_timeval64_to_timeval_long (new_value->it_interval);
  new_value_32.it_value
    = valid_timeval64_to_timeval_long (new_value->it_value);

  if (old_value == NULL)
    return INLINE_SYSCALL_CALL (setitimer, which, &new_value_32, NULL);

  struct itimerval32 old_value_32;
  if (INLINE_SYSCALL_CALL (setitimer, which, &new_value_32, &old_value_32) == -1)
    return -1;

  /* Write all fields of 'old_value' regardless of overflow.  */
  old_value->it_interval
     = valid_timeval_long_to_timeval64 (old_value_32.it_interval);
  old_value->it_value
     = valid_timeval_long_to_timeval64 (old_value_32.it_value);
  return 0;
#else
  return INLINE_SYSCALL_CALL (setitimer, which, new_value, old_value);
#endif
}

weak_alias (__setitimer, setitimer)
