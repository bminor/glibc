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
#include <stddef.h>
#include <unistd.h>

/* Test for access to FILE by our real user and group IDs.  */
int
DEFUN(__access, (file, type), CONST char *file AND int type)
{
  error_t err;
  const char *name;
  file_t rdir, dir, file;
  int flags;

  dir = __hurd_path_split (file, &name);
  if (dir == MACH_PORT_NULL)
    return -1;

  /* Set up _hurd_rid_auth.  */
  __mutex_lock (&_hurd_idlock);
  if (!_hurd_id_valid)
    {
      if (_hurd_rid_auth != MACH_PORT_NULL)
	{
	  __mach_port_deallocate (__mach_task_self (), _hurd_rid_auth);
	  _hurd_rid_auth = MACH_PORT_NULL;
	}
      if (err = __auth_getids (_hurd_auth, &_hurd_id))
	goto lose;

    }
  if (_hurd_rid_auth == MACH_PORT_NULL)
    {
      idblock_t rid;
      rid = _hurd_id;
      /* XXX Should keep supplementary ids or not? */
      rid.uids[0] = rid.ruid;
      rid.gids[0] = rid.rgid;
      rid.ruid = _hurd_id.uids[0];
      rid.rgid = _hurd_id.gids[0];
      if (err = __auth_makeauth (_hurd_auth, &rid, &_hurd_rid_auth))
	goto lose;
    }

  if (err = __io_reauthenticate (dir))
    goto lose;

  err = __auth_user_authenticate (_hurd_rid_auth, dir, &rdir);
  __mutex_unlock (&_hurd_idlock);
  __mach_port_deallocate (__mach_task_self (), dir);
  if (err)
    return __hurd_fail (err);

  flags = 0;
  if (type & R_OK)
    flags |= FS_LOOKUP_READ;
  if (type & W_OK)
    flags |= FS_LOOKUP_WRITE;
  if (type & X_OK)
    flags |= FS_LOOKUP_EXECUTE;

  /* XXX auth for crdir on retries from /? */
  err = __dir_lookup (rdir, name, flags, &file);
  __mach_port_deallocate (__mach_task_self (), rdir);
  if (err)
    return __hurd_fail (err);

  __mach_port_deallocate (__mach_task_self (), file);
  return 0;

 lose:
  __mutex_unlock (&_hurd_idlock);
  __mach_port_deallocate (__mach_task_self (), dir);
  return __hurd_fail (err);
}
