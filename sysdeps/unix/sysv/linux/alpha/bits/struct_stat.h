/* Definition for struct stat.
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

#if !defined _SYS_STAT_H && !defined _FCNTL_H
# error "Never include <bits/struct_stat.h> directly; use <sys/stat.h> instead."
#endif

#ifndef _BITS_STRUCT_STAT_H
#define _BITS_STRUCT_STAT_H	1

/* Nanosecond resolution timestamps are stored in a format equivalent to
   'struct timespec'.  This is the type used whenever possible but the
   Unix namespace rules do not allow the identifier 'timespec' to appear
   in the <sys/stat.h> header.  Therefore we have to handle the use of
   this header in strictly standard-compliant sources special.

   Use neat tidy anonymous unions and structures when possible.  */

#ifdef __USE_XOPEN2K8
# if __GNUC_PREREQ(3,3)
#  define __ST_TIME(X)				\
	__extension__ union {			\
	    struct timespec st_##X##tim;	\
	    struct {				\
		__time_t st_##X##time;		\
		unsigned long st_##X##timensec;	\
	    };					\
	}
# else
#  define __ST_TIME(X) struct timespec st_##X##tim
#  define st_atime st_atim.tv_sec
#  define st_mtime st_mtim.tv_sec
#  define st_ctime st_ctim.tv_sec
# endif
#else
# define __ST_TIME(X)				\
	__time_t st_##X##time;			\
	unsigned long st_##X##timensec
#endif


struct stat
  {
    __dev_t st_dev;		/* Device.  */
#ifdef __USE_FILE_OFFSET64
    __ino64_t st_ino;		/* File serial number.  */
#else
    __ino_t st_ino;		/* File serial number.	*/
    int __pad0;			/* 64-bit st_ino.  */
#endif
    __dev_t st_rdev;		/* Device number, if device.  */
    __off_t st_size;		/* Size of file, in bytes.  */
#ifdef __USE_FILE_OFFSET64
    __blkcnt64_t st_blocks;	/* Nr. 512-byte blocks allocated.  */
#else
    __blkcnt_t st_blocks;	/* Nr. 512-byte blocks allocated.  */
    int __pad1;			/* 64-bit st_blocks.  */
#endif
    __mode_t st_mode;		/* File mode.  */
    __uid_t st_uid;		/* User ID of the file's owner.	*/
    __gid_t st_gid;		/* Group ID of the file's group.*/
    __blksize_t st_blksize;	/* Optimal block size for I/O.  */
    __nlink_t st_nlink;		/* Link count.  */
    int __pad2;			/* Real padding.  */
    __ST_TIME(a);		/* Time of last access.  */
    __ST_TIME(m);		/* Time of last modification.  */
    __ST_TIME(c);		/* Time of last status change.  */
    long __glibc_reserved[3];
  };

#ifdef __USE_LARGEFILE64
/* Note stat64 is the same shape as stat.  */
struct stat64
  {
    __dev_t st_dev;		/* Device.  */
    __ino64_t st_ino;		/* File serial number.  */
    __dev_t st_rdev;		/* Device number, if device.  */
    __off_t st_size;		/* Size of file, in bytes.  */
    __blkcnt64_t st_blocks;	/* Nr. 512-byte blocks allocated.  */
    __mode_t st_mode;		/* File mode.  */
    __uid_t st_uid;		/* User ID of the file's owner.	*/
    __gid_t st_gid;		/* Group ID of the file's group.*/
    __blksize_t st_blksize;	/* Optimal block size for I/O.  */
    __nlink_t st_nlink;		/* Link count.  */
    int __pad0;			/* Real padding.  */
    __ST_TIME(a);		/* Time of last access.  */
    __ST_TIME(m);		/* Time of last modification.  */
    __ST_TIME(c);		/* Time of last status change.  */
    long __glibc_reserved[3];
  };
#endif

#undef __ST_TIME

/* Tell code we have these members.  */
#define	_STATBUF_ST_BLKSIZE
#define _STATBUF_ST_RDEV
#define _STATBUF_ST_NSEC


#endif /* _BITS_STRUCT_STAT_H  */
