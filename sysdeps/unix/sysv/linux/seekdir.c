/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <dirent.h>
#include <unistd.h>
#include <dirstream.h>

/* Seek to position POS in DIRP.  */
void
seekdir (DIR *dirp, long int pos)
{
  off64_t filepos;

  __libc_lock_lock (dirp->lock);

#if _DIRENT_OFFSET_TRANSLATION
  union dirstream_packed dsp = { .l = pos };
  if (dsp.p.is_packed == 1)
    filepos = dsp.p.info;
  else
    {
      size_t index = dsp.p.info;

      if (index >= dirstream_loc_size (&dirp->locs))
	{
	  __libc_lock_unlock (dirp->lock);
	  return;
	}
      filepos = *dirstream_loc_at (&dirp->locs, index);
    }
#else
  filepos = pos;
#endif

  __lseek64 (dirp->fd, filepos, SEEK_SET);
  dirp->filepos = filepos;
  dirp->offset = 0;
  dirp->size = 0;

  __libc_lock_unlock (dirp->lock);
}
