/* Copyright (C) 1991 Free Software Foundation, Inc.
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

/* Get the effective user ID of the calling process.  */
uid_t
DEFUN_VOID(__geteuid)
{
  uid_t euid;
  __mutex_lock (&_hurd_idlock);
  if (!_hurd_id_valid)
    {
      error_t err = __auth_getids (_hurd_auth, &_hurd_id);
      if (err)
	{
	  __mutex_unlock (&_hurd_idlock);
	  errno = __hurd_errno (err);
	  return -1;
	}
      _hurd_id_valid = 1;
    }
  if (_hurd_id.nuids == 0)
    {
      errno = ENOENT;
      euid = -1;
    }
  else
    euid = _hurd_id.uids[0];
  __mutex_unlock (&_hurd_idlock);
  return euid;
}
