/* time -- Get number of seconds since Epoch.  Linux/x86 version.
   Copyright (C) 2015-2020 Free Software Foundation, Inc.
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

#include <time.h>
#include <sysdep.h>
#include <sysdep-vdso.h>

static time_t
time_vsyscall (time_t *t)
{
#ifdef HAVE_TIME_VSYSCALL
  return INLINE_VSYSCALL (time, 1, t);
#else
  return INLINE_SYSCALL_CALL (time, t);
#endif
}

#ifdef SHARED
# include <dl-vdso.h>
# include <libc-vdso.h>

#undef INIT_ARCH
#define INIT_ARCH()
/* If the vDSO is not available we fall back on the syscall.  */
libc_ifunc (time, (get_vdso_symbol ("__vdso_time") ?: time_vsyscall))
#else
time_t
time (time_t *t)
{
  return time_vsyscall (t);
}
#endif
