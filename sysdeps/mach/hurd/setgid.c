/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

/* Set the group ID of the calling process to UID.
   If the calling process is the super-group, the real
   and effective group IDs, and the saved set-group-ID to UID;
   if not, the effective group ID is set to GID.  */
int
DEFUN(__setgid, (gid), gid_t gid)
{
  auth_t newauth;
  int i;

  __mutex_lock (&_hurd_idlock);
  if (!_hurd_id_valid)
    {
      error_t err = _HURD_PORT_USE (&_hurd_auth,
				    __auth_getids (port, &_hurd_id));
      if (err)
	{
	  __mutex_unlock (&_hurd_idlock);
	  return __hurd_fail (err);
	}
      _hurd_id_valid = 1;
    }

  for (i = 0; i < _hurd_id.ngroups; ++i)
    if (_hurd_id.gids[i] == gid)
      {
	/* We already have this gid.  Swap it with gids[0]
	   so getgid will return it.  */
	_hurd_id.gids[i] = _hurd_id.gids[0];
	break;
      }

  if (i == _hurd_id.ngroups)
    {
      if (_hurd_id.ngroups == (sizeof (_hurd_id.gids) /
			       sizeof (_hurd_id.gids[0])))
	{
	  __mutex_unlock (&_hurd_idlock);
	  errno = ENOMEM;	/* XXX ? */
	  return -1;
	}
      else
	{
	  _hurd_id.gids[_hurd_id.ngroups++] = _hurd_id.gids[0];
	  _hurd_id.gids[0] = gid;
	}
    }

  _hurd_id.rgid = _hurd_id.gids[0];

  if (err = _HURD_PORT_USE (&_hurd_auth,
			    __auth_makeauth (port, &_hurd_id, &newauth)))
    {
      _hurd_id_valid = 0;
      __mutex_unlock (&_hurd_idlock);
      return __hurd_fail (err);
    }
  else
    {
      int err = __setauth (newauth);
      __mach_port_deallocate (__mach_task_self (), newauth);
      return err;
    }
}
