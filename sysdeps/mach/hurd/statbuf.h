/* Copyright (C) 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_STATBUF_H

#define	_STATBUF_H	1

#include <gnu/types.h>

struct stat
  {
    int st_ftype;		/* File system type.  */
    __fsid_t st_fsid;		/* File system ID.  */
    __ino_t st_ino;		/* File number.  */
    unsigned int st_gen;	/* To detect reuse of file numbers.  */
    __dev_t st_rdev;		/* Device if special file.  */
    __mode_t st_mode;		/* File mode.  */
    __nlink_t st_nlink;		/* Number of links.  */
    __uid_t st_uid;		/* Owner.  */
    __gid_t st_gid;		/* Owning group.  */
    __off_t st_size;		/* Size in bytes.  */
    __time_t st_atime;		/* Access time, seconds */
    unsigned long int st_atime_usec; /* and microseconds.  */
    __time_t st_mtime;		/* Modification time, seconds */
    unsigned long int st_mtime_usec; /* and microseconds.  */
    __time_t st_ctime;		/* Status change time, seconds */
    unsigned long int st_ctime_usec; /* and microseconds.  */
    __uid_t st_author;		/* File author.  */
    int st_spare[8];		/* Reserved for future use.  */
  };

#endif /* statbuf.h */
