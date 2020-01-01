/* Copyright (C) 2003-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <shlib-compat.h>
#include <sysdep.h>
#include "kernel-posix-timers.h"
#include "compat-timer.h"

int
__timer_gettime_new (timer_t timerid, struct itimerspec *value)
{
  struct timer *kt = (struct timer *) timerid;

  return INLINE_SYSCALL_CALL (timer_gettime, kt->ktimerid, value);
}
versioned_symbol (librt, __timer_gettime_new, timer_gettime, GLIBC_2_3_3);


#if SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_3_3)
int
__timer_gettime_old (int timerid, struct itimerspec *value)
{
  return __timer_gettime_new (__compat_timer_list[timerid], value);
}
compat_symbol (librt, __timer_gettime_old, timer_gettime, GLIBC_2_2);
#endif
