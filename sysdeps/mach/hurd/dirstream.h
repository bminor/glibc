/* Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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

#ifndef	_DIRSTREAM_H

#define	_DIRSTREAM_H	1

#include <gnu/types.h>

/* Directory stream type.

   The Hurd directory format is the same as `struct dirent', so `readdir'
   returns a pointer into the buffer we read directory data into.  */

typedef struct
  {
    /* XXX we need a namespace-clean name for mach_port_t! */
    unsigned int __port;	/* Port to the directory.  */
    __off_t __filepos;		/* Current file position.  */
    __off_t __block_size;	/* Size to read chunks in (st_blksize).  */
    char *__data;		/* Directory block.  */
    unsigned int __allocation;	/* Space allocated for the block.  */
    unsigned int __size;	/* Total valid data in the block.  */
    __off_t __offset;		/* Current offset into the block.  */
  } DIR;

#endif	/* dirstream.h */
