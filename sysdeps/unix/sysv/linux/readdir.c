/* Read a directory.  Linux no-LFS version.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.
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
#include <dirstream.h>
#include <unistd.h>

# ifndef DIRENT_SET_DP_INO
#  define DIRENT_SET_DP_INO(dp, value) (dp)->d_ino = (value)
# endif

/* Read a directory entry from DIRP.  */
struct dirent *
__readdir_unlocked (DIR *dirp)
{
  int saved_errno = errno;

  while (1)
    {
      if (dirp->offset >= dirp->size)
	{
	  ssize_t bytes = __getdents64 (dirp->fd, dirp->data,
					dirp->allocation);
	  if (bytes <= 0)
	    {
	      /* Linux may fail with ENOENT on some file systems if the
		 directory inode is marked as dead (deleted).  POSIX
		 treats this as a regular end-of-directory condition, so
		 do not set errno in that case, to indicate success.  */
	      if (bytes < 0 && errno == ENOENT)
		__set_errno (saved_errno);
	      return NULL;
	    }
	  dirp->size = bytes;

 	  /* Reset the offset into the buffer.  */
	  dirp->offset = 0;
 	}

    /* These two pointers might alias the same memory buffer.  Standard C
       requires that we always use the same type for them, so we must use the
       union type.  */
      union
      {
	struct dirent64 dp64;
	struct dirent dp;
	char *b;
      } *inp, *outp;
      inp = (void*) &dirp->data[dirp->offset];
      outp = (void*) &dirp->data[dirp->offset];

      const size_t size_diff = offsetof (struct dirent64, d_name)
	- offsetof (struct dirent, d_name);

      /* Since inp->dp64.d_reclen is already aligned for the kernel structure
	 this may compute a value that is bigger than necessary.  */
      size_t old_reclen = inp->dp64.d_reclen;
      size_t new_reclen = ALIGN_UP (old_reclen - size_diff,
				    _Alignof (struct dirent));

      /* telldir can not return an error, so preallocate a map entry if
	 d_off can not be used directly.  */
      if (telldir_need_dirstream (inp->dp64.d_off))
	{
	  dirstream_loc_add (&dirp->locs, inp->dp64.d_off);
	  if (dirstream_loc_has_failed (&dirp->locs))
	    return NULL;
	}

      /* Copy the data from INP and access only OUTP.  */
      const uint64_t d_ino = inp->dp64.d_ino;
      const int64_t d_off = inp->dp64.d_off;
      const uint8_t d_type = inp->dp64.d_type;
      /* This will clamp both d_off and d_ino values, which is required to
	 avoid return EOVERFLOW.  The lelldir/seekdir uses the 'locs' value
	 if the value overflows.  */
      outp->dp.d_ino = d_ino;
      outp->dp.d_off = d_off;
      outp->dp.d_reclen = new_reclen;
      outp->dp.d_type = d_type;
      memmove (outp->dp.d_name, inp->dp64.d_name,
	       old_reclen - offsetof (struct dirent64, d_name));

      dirp->filepos = d_off;
      dirp->offset += old_reclen;

      return &outp->dp;
    }
}

struct dirent *
__readdir (DIR *dirp)
{
  struct dirent *dp;

#if IS_IN (libc)
  __libc_lock_lock (dirp->lock);
#endif
  dp = __readdir_unlocked (dirp);
#if IS_IN (libc)
  __libc_lock_unlock (dirp->lock);
#endif

  return dp;
}
weak_alias (__readdir, readdir)

#endif
