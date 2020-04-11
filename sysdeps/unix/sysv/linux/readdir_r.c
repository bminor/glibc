/* Read a directory in reentrant mode.  Linux no-LFS version.
   Copyright (C) 2018-2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <dirent.h>

#if !_DIRENT_MATCHES_DIRENT64
/* Read a directory entry from DIRP.  */
int
__readdir_r (DIR *dirp, struct dirent *entry, struct dirent **result)
{
  struct dirent *dp;
  size_t reclen;
  const int saved_errno = errno;
  int ret;

  __libc_lock_lock (dirp->lock);

  do
    {
      if (dirp->offset >= dirp->size)
	{
	  /* We've emptied out our buffer.  Refill it.  */

	  size_t maxread = dirp->allocation;
	  ssize_t bytes;

	  maxread = dirp->allocation;

	  bytes = __getdents (dirp->fd, dirp->data, maxread);
	  if (bytes <= 0)
	    {
	      /* On some systems getdents fails with ENOENT when the
		 open directory has been rmdir'd already.  POSIX.1
		 requires that we treat this condition like normal EOF.  */
	      if (bytes < 0 && errno == ENOENT)
		{
		  bytes = 0;
		  __set_errno (saved_errno);
		}
	      if (bytes < 0)
		dirp->errcode = errno;

	      dp = NULL;
	      break;
	    }
	  dirp->size = (size_t) bytes;

	  /* Reset the offset into the buffer.  */
	  dirp->offset = 0;
	}

      dp = (struct dirent *) &dirp->data[dirp->offset];

      reclen = dp->d_reclen;

      dirp->offset += reclen;

      dirp->filepos = dp->d_off;

      if (reclen > offsetof (struct dirent, d_name) + NAME_MAX + 1)
	{
	  /* The record is very long.  It could still fit into the
	     caller-supplied buffer if we can skip padding at the
	     end.  */
	  size_t namelen = _D_EXACT_NAMLEN (dp);
	  if (namelen <= NAME_MAX)
	    reclen = offsetof (struct dirent, d_name) + namelen + 1;
	  else
	    {
	      /* The name is too long.  Ignore this file.  */
	      dirp->errcode = ENAMETOOLONG;
	      dp->d_ino = 0;
	      continue;
	    }
	}

      /* Skip deleted and ignored files.  */
    }
  while (dp->d_ino == 0);

  if (dp != NULL)
    {
      *result = memcpy (entry, dp, reclen);
      entry->d_reclen = reclen;
      ret = 0;
    }
  else
    {
      *result = NULL;
      ret = dirp->errcode;
    }

  __libc_lock_unlock (dirp->lock);

  return ret;
}

weak_alias (__readdir_r, readdir_r)
#endif /* _DIRENT_MATCHES_DIRENT64  */
