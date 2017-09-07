/* Get the current value of an interval timer

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

#include <stddef.h>
#include <errno.h>
#include <sys/time.h>

/* Set *VALUE to the current setting of timer WHICH.
   Return 0 on success, -1 on errors.  */
int
__getitimer64 (enum __itimer_which which,
                 struct __itimerval64 *value)
{
  struct itimerval value32, *value32p= NULL;

  if (value != NULL)
    value32p = &value32;

  int result = __getitimer(which, value32p);

  if (result == 0 && value != NULL)
    {
      value->it_interval.tv_sec = value32.it_interval.tv_sec;
      value->it_interval.tv_usec = value32.it_interval.tv_usec;
      value->it_value.tv_sec = value32.it_value.tv_sec;
      value->it_value.tv_usec = value32.it_value.tv_usec;
    }

  return result;
}
