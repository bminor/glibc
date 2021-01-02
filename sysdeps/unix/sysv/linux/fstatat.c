/* Get file status.  Linux version.
   Copyright (C) 2020-2021 Free Software Foundation, Inc.
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

#include <sys/stat.h>
#include <kernel_stat.h>
#include <sysdep.h>

#if !XSTAT_IS_XSTAT64
# include <kstat_cp.h>

int
__fstatat (int fd, const char *file, struct stat *buf, int flag)
{
  int r;

# if STAT_IS_KERNEL_STAT
  /* New kABIs which uses generic pre 64-bit time Linux ABI, e.g.
     csky, nios2  */
  r = INTERNAL_SYSCALL_CALL (fstatat64, fd, file, buf, flag);
  if (r == 0 && (buf->__st_ino_pad != 0
		 || buf->__st_size_pad != 0
		 || buf->__st_blocks_pad != 0))
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (EOVERFLOW);
# else
#  ifdef __NR_fstatat64
  /* Old KABIs with old non-LFS support, e.g. arm, i386, hppa, m68k, mips32,
     microblaze, s390, sh, powerpc, and sparc.  */
  struct stat64 st64;
  r = INTERNAL_SYSCALL_CALL (fstatat64, fd, file, &st64, flag);
  if (r == 0)
    {
      if (! in_ino_t_range (st64.st_ino)
	  || ! in_off_t_range (st64.st_size)
	  || ! in_blkcnt_t_range (st64.st_blocks))
	return INLINE_SYSCALL_ERROR_RETURN_VALUE (EOVERFLOW);

      /* Clear internal pad and reserved fields.  */
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
      buf->st_atim.tv_sec = st64.st_atim.tv_sec;
      buf->st_atim.tv_nsec = st64.st_atim.tv_nsec;
      buf->st_mtim.tv_sec = st64.st_mtim.tv_sec;
      buf->st_mtim.tv_nsec = st64.st_mtim.tv_nsec;
      buf->st_ctim.tv_sec = st64.st_ctim.tv_sec;
      buf->st_ctim.tv_nsec = st64.st_ctim.tv_nsec;

      return 0;
    }
#  else
  /* 64-bit kabi outlier, e.g. mips64 and mips64-n32.  */
  struct kernel_stat kst;
  r = INTERNAL_SYSCALL_CALL (newfstatat, fd, file, &kst, flag);
  if (r == 0)
    r = __cp_kstat_stat (&kst, buf);
#  endif /* __nr_fstatat64  */
# endif /* STAT_IS_KERNEL_STAT  */

  return INTERNAL_SYSCALL_ERROR_P (r)
	 ? INLINE_SYSCALL_ERROR_RETURN_VALUE (-r)
	 : 0;
}

weak_alias (__fstatat, fstatat)
#endif
