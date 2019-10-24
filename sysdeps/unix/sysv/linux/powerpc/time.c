/* time system call for Linux/PowerPC.
   Copyright (C) 2013-2019 Free Software Foundation, Inc.
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

#ifdef HAVE_TIME_VSYSCALL
# define HAVE_VSYSCALL
#endif
#include <sysdep-vdso.h>

static time_t
time_vsyscall (time_t *t)
{
  return INLINE_VSYSCALL (time, 1, t);
}

#ifdef SHARED
# include <dl-vdso.h>
# include <libc-vdso.h>

# define INIT_ARCH() \
  void *vdso_time = get_vdso_symbol (HAVE_TIME_VSYSCALL);

/* If the vDSO is not available we fall back to the syscall.  */
libc_ifunc (time,
	    vdso_time
	    ? VDSO_IFUNC_RET (vdso_time)
	    : (void *) time_vsyscall);

#else
time_t
time (time_t *t)
{
  return time_vsyscall (t);
}
#endif /* !SHARED */
