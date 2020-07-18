/* lxstat using old-style Unix stat system call.
   Copyright (C) 2004-2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#define __lxstat __redirect___lxstat
#include <sys/stat.h>
#undef __lxstat
#include <fcntl.h>
#include <kernel_stat.h>
#include <sysdep.h>
#include <xstatconv.h>
#include <shlib-compat.h>

/* Get information about the file NAME in BUF.  */
int
attribute_compat_text_section
__lxstat64 (int vers, const char *name, struct stat64 *buf)
{
  switch (vers)
    {
    case _STAT_VER_KERNEL64:
      return INLINE_SYSCALL_CALL (lstat64, name, buf);

    default:
      {
        struct kernel_stat kbuf;
	int r = INTERNAL_SYSCALL_CALL (lstat, name, &kbuf);
	if (r == 0)
	  return __xstat_conv (vers, &kbuf, buf);
	return INLINE_SYSCALL_ERROR_RETURN_VALUE (-r);
      }
    }
}

#if SHLIB_COMPAT(libc, GLIBC_2_0, GLIBC_2_33)
strong_alias (__lxstat64, __lxstat_compat)
compat_symbol (libc, __lxstat_compat, __lxstat, GLIBC_2_0);
#endif

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_33)
compat_symbol (libc, __lxstat64, __lxstat64, GLIBC_2_1);
#endif
