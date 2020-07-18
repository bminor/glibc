/* Struct stat/stat64 to stat/stat64 conversion for Linux.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <sys/stat.h>
#include <kernel_stat.h>

static inline int
__cp_kstat_stat (const struct kernel_stat *kst, struct stat *st)
{
  st->st_dev = kst->st_dev;
  memset (&st->st_pad1, 0, sizeof (st->st_pad1));
  st->st_ino = kst->st_ino;
  if (st->st_ino != kst->st_ino)
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (EOVERFLOW);
  st->st_mode = kst->st_mode;
  st->st_nlink = kst->st_nlink;
  st->st_uid = kst->st_uid;
  st->st_gid = kst->st_gid;
  st->st_rdev = kst->st_rdev;
  memset (&st->st_pad2, 0, sizeof (st->st_pad2));
  st->st_size = kst->st_size;
  if (st->st_size != kst->st_size)
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (EOVERFLOW);
  st->st_pad3 = 0;
  st->st_atim.tv_sec = kst->st_atime_sec;
  st->st_atim.tv_nsec = kst->st_atime_nsec;
  st->st_mtim.tv_sec = kst->st_mtime_sec;
  st->st_mtim.tv_nsec = kst->st_mtime_nsec;
  st->st_ctim.tv_sec = kst->st_ctime_sec;
  st->st_ctim.tv_nsec = kst->st_ctime_nsec;
  st->st_blksize = kst->st_blksize;
  st->st_blocks = kst->st_blocks;
  if (st->st_blocks != kst->st_blocks)
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (EOVERFLOW);
  memset (&st->st_pad5, 0, sizeof (st->st_pad5));

  return 0;
}

static inline int
__cp_kstat_stat64 (const struct kernel_stat *kst, struct stat64 *st)
{
  st->st_dev = kst->st_dev;
  memset (&st->st_pad1, 0, sizeof (st->st_pad1));
  st->st_ino = kst->st_ino;
  st->st_mode = kst->st_mode;
  st->st_nlink = kst->st_nlink;
  st->st_uid = kst->st_uid;
  st->st_gid = kst->st_gid;
  st->st_rdev = kst->st_rdev;
  memset (&st->st_pad2, 0, sizeof (st->st_pad2));
  st->st_pad3 = 0;
  st->st_size = kst->st_size;
  st->st_blksize = kst->st_blksize;
  st->st_blocks = kst->st_blocks;
  st->st_atim.tv_sec = kst->st_atime_sec;
  st->st_atim.tv_nsec = kst->st_atime_nsec;
  st->st_mtim.tv_sec = kst->st_mtime_sec;
  st->st_mtim.tv_nsec = kst->st_mtime_nsec;
  st->st_ctim.tv_sec = kst->st_ctime_sec;
  st->st_ctim.tv_nsec = kst->st_ctime_nsec;
  memset (&st->st_pad4, 0, sizeof (st->st_pad4));

  return 0;
}
