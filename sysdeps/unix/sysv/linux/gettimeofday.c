/* Copyright (C) 2015-2019 Free Software Foundation, Inc.

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
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/time.h>

#undef __gettimeofday

#ifdef HAVE_GETTIMEOFDAY_VSYSCALL
# define HAVE_VSYSCALL
#endif
#include <sysdep-vdso.h>

/* Get the current time of day and timezone information,
   putting it into *tv and *tz.  If tz is null, *tz is not filled.
   Returns 0 on success, -1 on errors.  */
int
___gettimeofday (struct timeval *tv, struct timezone *tz)
{
  return INLINE_VSYSCALL (gettimeofday, 2, tv, tz);
}

#ifdef VERSION_gettimeofday
weak_alias (___gettimeofday, __wgettimeofday);
default_symbol_version (___gettimeofday, __gettimeofday, VERSION_gettimeofday);
default_symbol_version (__wgettimeofday,   gettimeofday, VERSION_gettimeofday);
libc_hidden_ver (___gettimeofday, __gettimeofday);
libc_hidden_ver (___gettimeofday, gettimeofday);
#else
strong_alias (___gettimeofday, __gettimeofday)
weak_alias (___gettimeofday, gettimeofday)
libc_hidden_def (__gettimeofday)
libc_hidden_weak (gettimeofday)
#endif
