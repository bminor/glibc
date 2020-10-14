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
#include <fcntl.h>
#include <string.h>
#include <kernel_stat.h>
#include <sysdep.h>
#include <time.h>
#include <statx_cp.h>
#include <kstat_cp.h>
#include <stat_t64_cp.h>

int
__fstatat64_time64 (int fd, const char *file, struct __stat64_t64 *buf,
		    int flag)
{
  int r;

#if (__WORDSIZE == 32 \
     && (!defined __SYSCALL_WORDSIZE || __SYSCALL_WORDSIZE == 32))
  /* 32-bit kABI with default 64-bit time_t, e.g. arc, riscv32.   Also
     64-bit time_t support is done through statx syscall.  */
  struct statx tmp;
  r = INTERNAL_SYSCALL_CALL (statx, fd, file, AT_NO_AUTOMOUNT | flag,
			     STATX_BASIC_STATS, &tmp);
  if (r == 0)
    {
      __cp_stat64_t64_statx (buf, &tmp);
      return 0;
    }
  if (-r != ENOSYS)
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (-r);
#endif

#if XSTAT_IS_XSTAT64
# ifdef __NR_newfstatat
  /* 64-bit kABI, e.g. aarch64, ia64, powerpc64*, s390x, riscv64, and
     x86_64.  */
  r = INTERNAL_SYSCALL_CALL (newfstatat, fd, file, buf, flag);
# elif defined __NR_fstatat64
#  if STAT64_IS_KERNEL_STAT64
  /* 64-bit kABI outlier, e.g. alpha  */
  r = INTERNAL_SYSCALL_CALL (fstatat64, fd, file, buf, flag);
#  else
  /* 64-bit kABI outlier, e.g. sparc64.  */
  struct kernel_stat64 kst64;
  r = INTERNAL_SYSCALL_CALL (fstatat64, fd, file, &kst64, flag);
  if (r == 0)
    __cp_stat64_kstat64 (buf, &kst64);
#  endif
# endif
#else
# ifdef __NR_fstatat64
  /* All kABIs with non-LFS support and with old 32-bit time_t support
     e.g. arm, csky, i386, hppa, m68k, microblaze, nios2, sh, powerpc32,
     and sparc32.  */
  struct stat64 st64;
  r = INTERNAL_SYSCALL_CALL (fstatat64, fd, file, &st64, flag);
  if (r == 0)
    {
      /* Clear both pad and reserved fields.  */
      memset (buf, 0, sizeof (*buf));

      buf->st_dev = st64.st_dev,
      buf->st_ino = st64.st_ino;
      buf->st_mode = st64.st_mode;
      buf->st_nlink = st64.st_nlink;
      buf->st_uid = st64.st_uid;
      buf->st_gid = st64.st_gid;
      buf->st_rdev = st64.st_rdev;
      buf->st_size = st64.st_size;
      buf->st_blksize = st64.st_blksize;
      buf->st_blocks  = st64.st_blocks;
      buf->st_atim = valid_timespec_to_timespec64 (st64.st_atim);
      buf->st_mtim = valid_timespec_to_timespec64 (st64.st_mtim);
      buf->st_ctim = valid_timespec_to_timespec64 (st64.st_ctim);
    }
# else
  /* 64-bit kabi outlier, e.g. mips64 and mips64-n32.  */
  struct kernel_stat kst;
  r = INTERNAL_SYSCALL_CALL (newfstatat, fd, file, &kst, flag);
  if (r == 0)
    __cp_kstat_stat64_t64 (&kst, buf);
# endif
#endif

  return INTERNAL_SYSCALL_ERROR_P (r)
	 ? INLINE_SYSCALL_ERROR_RETURN_VALUE (-r)
	 : 0;
}
#if __TIMESIZE != 64
hidden_def (__fstatat64_time64)

int
__fstatat64 (int fd, const char *file, struct stat64 *buf, int flags)
{
  struct __stat64_t64 st_t64;
  return __fstatat64_time64 (fd, file, &st_t64, flags)
	 ?: __cp_stat64_t64_stat64 (&st_t64, buf);
}
#endif

#undef __fstatat
#undef fstatat

hidden_def (__fstatat64)
weak_alias (__fstatat64, fstatat64)

#if XSTAT_IS_XSTAT64
strong_alias (__fstatat64, __fstatat)
weak_alias (__fstatat64, fstatat)
strong_alias (__fstatat64, __GI___fstatat);
#endif
