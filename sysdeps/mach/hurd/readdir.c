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
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <hurd.h>


/* Read a directory entry from DIRP.  */
struct dirent *
DEFUN(readdir, (dirp), DIR *dirp)
{
  struct dirent *dp;

  if (dirp == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  do
    {
      if (dirp->__offset >= dirp->__size)
	{
	  /* We've emptied out our buffer.  Refill it.  */

	  char *data = dirp->__data;
	  error_t err = __dir_readdir (dirp->__port,
				       &data, &dirp->__size,
				       dirp->__filepos, &dirp->__filepos,
				       dirp->__block_size);
	  if (err)
	    {
	      errno = err;
	      return NULL;
	    }

	  if (data != dirp->__data)
	    {
	      /* The data was passed out of line, so our old buffer is no
		 longer useful.  Deallocate the old buffer and reset our
		 information for the new buffer.  */
	      __vm_deallocate (__mach_task_self (),
			       (vm_address_t) dirp->__data,
			       dirp->__allocation);
	      dirp->__data = data;
	      dirp->__allocation = dirp->__size;
	    }

	  /* Reset the offset into the buffer.  */
	  dirp->__offset = 0;
	}

      dp = (struct dirent *) &dirp->__data[dirp->__offset];
      dirp->__offset += dp->d_reclen;

      /* Loop to ignore deleted files.  */
    } while (dp->d_fileno == 0);

  return dp;
}
