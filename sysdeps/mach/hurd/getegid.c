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
#include <unistd.h>
#include <hurd.h>

/* Get the effective group ID of the calling process.  */
gid_t
DEFUN_VOID(__getegid)
{
  error_t err;
  gid_t egid;

  __mutex_lock (&_hurd_idlock);

  if (err = _hurd_check_ids ())
    {
      errno = err;
      egid = -1;
    }
  else
    switch (_hurd_ngids)
      {
      case 0:
	/* We have not even a real gid.  */
	errno = XXX;
	egid = -1;
	break;
      case 1:
      case 2:
	/* We have no effective gids.  Return the real gid.  */
	egid = _hurd_gid.rid;
	break;
      default:
	egid = _hurd_gid.ids[0];
	break;
      }

  __mutex_unlock (&_hurd_idlock);
  return egid;
}
