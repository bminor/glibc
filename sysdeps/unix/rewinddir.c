/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility for
the consequences of using it or for whether it serves any particular
purpose or works at all, unless he says so in writing.  Refer to the GNU
C Library General Public License (in the file COPYING) for full details.

Everyone is granted permission to copy, modify and redistribute
the GNU C Library, but only under the conditions described in the
GNU C Library General Public License.  Among other things, this notice
must not be changed and a copy of the license must be included.  */

#include <ansidecl.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>


/* Rewind DIRP to the beginning of the directory.  */
void
DEFUN(rewinddir, (dirp), DIR *dirp)
{
  (void) lseek(dirp->__fd, (off_t) 0, SEEK_SET);
  dirp->__offset = 0;
  dirp->__size = 0;
}
