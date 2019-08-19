/* settimeofday -- Set the current time of day.  Linux/Alpha/tv32 version.
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

#include <shlib-compat.h>

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)

#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <tv32-compat.h>

/* Set the current time of day and timezone information.
   This call is restricted to the super-user.  */
int
attribute_compat_text_section
__settimeofday_tv32 (const struct timeval32 *tv32,
                     const struct timezone *tz)
{
  struct timeval tv = valid_timeval_to_timeval64 (*tv32);
  return __settimeofday (&tv, tz);
}

compat_symbol (libc, __settimeofday_tv32, settimeofday, GLIBC_2_0);
#endif
