/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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
   If the calling process is the super-user, the real
   and effective group IDs, and the saved set-group-ID to UID;
   if not, the effective group ID is set to GID.  */
int
DEFUN(__setgid, (gid), gid_t gid)
{
  auth_t newauth;
  int i;
  error_t err;

  __mutex_lock (&_hurd_idlock);
  err = _hurd_check_ids ();

  for (i = 0; i < _hurd_ngids - 2; ++i)
    if (_hurd_gid->ids[i] == gid)
      {
	/* We already have this gid.
	   Swap it with gids[0] so getegid will return it.  */
	_hurd_gid->ids[i] = _hurd_gid->ids[0];
	break;
      }

  /* XXX unfinished */

  _hurd_gid->rid = gid;

  err = _HURD_PORT_USE (&_hurd_auth,
			__auth_makeauth (port, &_hurd_id, &newauth));
  _hurd_id_valid = 0;
  __mutex_unlock (&_hurd_idlock);

  if (err)
    return __hurd_fail (err);

  err = __setauth (newauth);
  __mach_port_deallocate (__mach_task_self (), newauth);
  return err;
}
