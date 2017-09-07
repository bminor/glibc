/* Copyright (C) 2015-2018 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/time.h>

#undef __gettimeofday

#ifdef HAVE_GETTIMEOFDAY_VSYSCALL
# define HAVE_VSYSCALL
#include <sysdep-vdso.h>
#endif

/* Get the current time of day and timezone information,
   putting it into *tv and *tz.  If tz is null, *tz is not filled.
   Returns 0 on success, -1 on errors.  */

int
__gettimeofday64 (struct __timeval64 *tv, struct timezone *tz)
{
  struct timeval tv32;
  int result;

#ifdef __vdso_gettimeofday
  result = INLINE_VSYSCALL (gettimeofday, 2, &tv32, tz);
#else
  result = INLINE_SYSCALL (gettimeofday, 2, &tv32, tz);
#endif

  if (result == 0)
    {
      tv->tv_sec = tv32.tv_sec;
      tv->tv_usec = tv32.tv_usec;
    }

  return result;
}
