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
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <bsddir.h>

/* Open a directory stream on NAME.  */
DIR *
DEFUN(opendir, (name), CONST char *name)
{
  DIR *dirp;
  struct stat statbuf;
  int fd;

  fd = __open(name, O_RDONLY);
  if (fd < 0)
    return NULL;

  {
    int flags = FD_CLOEXEC;
    if (fcntl(fd, F_SETFD, &flags) < 0)
      return NULL;
  }

  dirp = (DIR *) malloc(sizeof(DIR) + NAME_MAX);
  if (dirp == NULL)
    {
      int save = errno;
      (void) __close(fd);
      errno = save;
      return NULL;
    }

  if (__fstat(fd, &statbuf) < 0 || statbuf.st_blksize < sizeof(struct direct))
    dirp->__allocation = sizeof(struct direct);
  else
    dirp->__allocation = statbuf.st_blksize;
  dirp->__data = (char *) malloc(dirp->__allocation);
  if (dirp->__data == NULL)
    {
      int save = errno;
      free((PTR) dirp);
      (void) __close(fd);
      errno = save;
      return NULL;
    }

  dirp->__fd = fd;
  dirp->__offset = 0;
  dirp->__size = 0;
  return dirp;
}
