/* Get file status.  Linux version.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#define __fstatat __redirect___fstatat
#define fstatat   __redirect_fstatat
#include <sys/stat.h>
#undef __fstatat
#undef fstatat
#include <fcntl.h>

#include <kernel_stat.h>
#include <sysdep.h>

#include <statx_cp.h>
#include <kstat_cp.h>

int
__fstatat64 (int fd, const char *file, struct stat64 *buf, int flag)
{
#if XSTAT_IS_XSTAT64
# ifdef __NR_newfstatat
  /* 64-bit kABI, e.g. aarch64, ia64, powerpc64*, s390x, riscv64, and
     x86_64.  */
  return INLINE_SYSCALL_CALL (newfstatat, fd, file, buf, flag);
# elif defined __NR_fstatat64
#  if STAT64_IS_KERNEL_STAT64
  /* 64-bit kABI outlier, e.g. alpha.  */
  return INLINE_SYSCALL_CALL (fstatat64, fd, file, buf, flag);
#  else
  /* 64-bit kABI outlier, e.g. sparc64.  */
  struct kernel_stat64 kst64;
  int r = INLINE_SYSCALL_CALL (fstatat64, fd, file, &kst64, flag);
  return r ?: __cp_stat64_kstat64 (buf, &kst64);
#  endif
# else
  /* 32-bit kABI with default 64-bit time_t, e.g. arc, riscv32.  */
  struct statx tmp;
  int r = INLINE_SYSCALL_CALL (statx, fd, file, AT_NO_AUTOMOUNT | flag,
			       STATX_BASIC_STATS, &tmp);
  if (r == 0)
    __cp_stat64_statx (buf, &tmp);
  return r;
# endif
#else
# ifdef __NR_fstatat64
  /* All kABIs with non-LFS support, e.g. arm, csky, i386, hppa, m68k,
     microblaze, nios2, sh, powerpc32, and sparc32.  */
  return INLINE_SYSCALL_CALL (fstatat64, fd, file, buf, flag);
# else
  /* 64-bit kabi outlier, e.g. mips64 and mips64-n32.  */
  struct kernel_stat kst;
  int r = INLINE_SYSCALL_CALL (newfstatat, fd, file, &kst, flag);
  return r ?: __cp_kstat_stat64 (&kst, buf);
# endif
#endif
}
hidden_def (__fstatat64)
weak_alias (__fstatat64, fstatat64)

#if XSTAT_IS_XSTAT64
strong_alias (__fstatat64, __fstatat)
weak_alias (__fstatat64, fstatat)
strong_alias (__fstatat64, __GI___fstatat);
#endif
