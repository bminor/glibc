/* Set an interval timer

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

/* Set the timer WHICH to *NEW.  If OLD is not NULL,
   set *OLD to the old value of timer WHICH.
   Returns 0 on success, -1 on errors.  */
int
__setitimer64 (enum __itimer_which which,
                 const struct __itimerval64 *new,
                 struct __itimerval64 *old)
{
  struct itimerval new32, *new32p = NULL;
  struct itimerval old32, *old32p = NULL;

  if (new != NULL)
    {
      if (new->it_interval.tv_sec > INT_MAX ||
          new->it_value.tv_sec > INT_MAX)
        {
          __set_errno (EOVERFLOW);
          return -1;
        }
      new32.it_interval.tv_sec = new->it_interval.tv_sec;
      new32.it_interval.tv_usec = new->it_interval.tv_usec;
      new32.it_value.tv_sec = new->it_value.tv_sec;
      new32.it_value.tv_usec = new->it_value.tv_usec;
      new32p = &new32;
    }

  if (old != NULL)
    old32p = &old32;

  int result = __setitimer(which, new32p, old32p);

  if (old)
    {
      old->it_interval.tv_sec = old32.it_interval.tv_sec;
      old->it_interval.tv_usec = old32.it_interval.tv_usec;
      old->it_value.tv_sec = old32.it_value.tv_sec;
      old->it_value.tv_usec = old32.it_value.tv_usec;
    }

  return result;
}
