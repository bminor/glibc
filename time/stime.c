/* Copyright (C) 1992-2025 Free Software Foundation, Inc.
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

#include <shlib-compat.h>

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_31)

#include <time.h>

/* Set the system clock to *WHEN.  */

int
attribute_compat_text_section
__stime (const time_t *when)
{
  struct timespec ts;
  ts.tv_sec = *when;
  ts.tv_nsec = 0;

  return __clock_settime (CLOCK_REALTIME, &ts);
}

compat_symbol (libc, __stime, stime, GLIBC_2_0);
#endif
