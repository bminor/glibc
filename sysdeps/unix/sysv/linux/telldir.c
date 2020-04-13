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

#include <stdio.h>
#include <assert.h>
#include <dirent.h>

#include <dirstream.h>
#include <telldir.h>

/* Return the current position of DIRP.  */
long int
telldir (DIR *dirp)
{
  long int ret;

  __libc_lock_lock (dirp->lock);

#if _DIRENT_OFFSET_TRANSLATION
  /* If the directory position fits in the packet structure, returns it.
     Otherwise, check if the position is already been recorded in the
     dynamic array.  If not, add the new record.  */

  union dirstream_packed dsp;

  if (!telldir_need_dirstream (dirp->filepos))
    {
      dsp.p.is_packed = 1;
      dsp.p.info = dirp->filepos;
    }
  else
    {
      dsp.l = -1;

      size_t i;
      for (i = 0; ;i++)
	{
	  /* It should be pre-allocated on readdir.  */
	  assert (i < dirstream_loc_size (&dirp->locs));
	  if (*dirstream_loc_at (&dirp->locs, i) == dirp->filepos)
	    break;
	}

      dsp.p.is_packed = 0;
      dsp.p.info = i;
    }

  ret = dsp.l;
#else
  ret = dirp->filepos;
#endif
  __libc_lock_unlock (dirp->lock);

  return ret;
}
