/* Copyright (C) 1993 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <hurd.h>


/* Open a directory stream on NAME.  */
DIR *
DEFUN(opendir, (name), CONST char *name)
{
  DIR *dirp;
  struct stat statbuf;
  file_t port;
  error_t err;

  port = __path_lookup (name, O_RDONLY, 0);
  if (port == MACH_PORT_NULL)
    return NULL;

  /* XXX this port should be deallocated on exec */

  if (err = __io_stat (port, &statbuf))
    {
      errno = err;
    lose:
      __mach_port_deallocate (__mach_task_self (), port);
      return NULL;
    }    

  dirp = (DIR *) malloc (sizeof (DIR));
  if (dirp == NULL)
    goto lose;

  dirp->__port = port;
  dirp->__filepos = 0;
  dirp->__block_size = statbuf.st_blksize;
  dirp->__data = NULL;
  dirp->__allocation = 0;
  dirp->__size = 0;
  dirp->__offset = 0;

  return dirp;
}
