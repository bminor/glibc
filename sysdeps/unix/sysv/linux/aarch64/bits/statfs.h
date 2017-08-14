/* Definition of `struct statfs', information about a filesystem.
   Copyright (C) 2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_STATFS_H
# error "Never include <bits/statfs.h> directly; use <sys/statfs.h> instead."
#endif

#include <endian.h>
#include <bits/types.h>
#include <bits/wordsize.h>

/* On AArch64 both the 32-bit and 64-bit libc's use the kernels
   'struct statfs' with 64 bit f_blocks/f_bfree/f_bavail/f_files/f_ffree
   fields.  This means the statfs and statfs64 structs are identical
   and the statfs and statfs64 calls can be aliases.  */

struct statfs
  {
    __SWORD_TYPE f_type;
    __SWORD_TYPE f_bsize;
    __fsblkcnt_t f_blocks;
    __fsblkcnt_t f_bfree;
    __fsblkcnt_t f_bavail;
    __fsfilcnt_t f_files;
    __fsfilcnt_t f_ffree;
    __fsid_t f_fsid;
    __SWORD_TYPE f_namelen;
    __SWORD_TYPE f_frsize;
    __SWORD_TYPE f_flags;
    __SWORD_TYPE f_spare[4];
  };

struct statfs64
  {
    __SWORD_TYPE f_type;
    __SWORD_TYPE f_bsize;
    __fsblkcnt64_t f_blocks;
    __fsblkcnt64_t f_bfree;
    __fsblkcnt64_t f_bavail;
    __fsfilcnt64_t f_files;
    __fsfilcnt64_t f_ffree;
    __fsid_t f_fsid;
    __SWORD_TYPE f_namelen;
    __SWORD_TYPE f_frsize;
    __SWORD_TYPE f_flags;
    __SWORD_TYPE f_spare[4];
  };

/* Tell code we have these members.  */
#define _STATFS_F_NAMELEN
#define _STATFS_F_FRSIZE
#define _STATFS_F_FLAGS
