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
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <bsddir.h>
#include <sys/types.h>

#ifndef	READ_DIRECTORY
#define	READ_DIRECTORY							      \
{									      \
  int bytes = __read(dirp->__fd, dirp->__data, dirp->__allocation);	      \
  if (bytes <= 0)							      \
    return NULL;							      \
  dirp->__size = (size_t) bytes;					      \
}
#endif

/* Read a directory entry from DIRP.  */
struct dirent *
DEFUN(readdir, (dirp), DIR *dirp)
{
  if (dirp == NULL || dirp->__data == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  for (;;)
    {
      struct direct *dp;

      if (dirp->__offset >= dirp->__size)
	{
	  /* We've emptied out our buffer.  Refill it.  */
	  READ_DIRECTORY;
	  /* Reset the offset into the buffer.  */
	  dirp->__offset = 0;
	}

      dp = (struct direct *) &dirp->__data[dirp->__offset];
      dirp->__offset += dp->d_reclen;

      if (dp->d_ino != 0)
	{
	  /* Not a deleted file.  */
	  register struct dirent *d = &dirp->__entry;
	  d->d_fileno = (ino_t) dp->d_ino;
	  d->d_namlen = (size_t) dp->d_namlen;
	  if (d->d_namlen > NAME_MAX)
	    d->d_namlen = NAME_MAX;
	  (void) strncpy(d->d_name, dp->d_name, d->d_namlen + 1);
	  return d;
	}
    }
}
