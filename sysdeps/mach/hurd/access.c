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
#include <unistd.h>
#include <hurd.h>

/* Test for access to FILE by our real user and group IDs.  */
int
DEFUN(__access, (file, type), CONST char *file AND int type)
{
  error_t err;
  file_t crdir, cwdir, rcrdir, rcwdir, io;
  int dealloc_crdir, dealloc_cwdir;
  int flags;

  __mutex_lock (&_hurd_id.lock);
  if (err = _hurd_check_ids ())
    {
      __mutex_unlock (&_hurd_id.lock);
      return __hurd_fail (err);
    }

  /* Set up _hurd_id.rid_auth.  */
  if (_hurd_id.rid_auth == MACH_PORT_NULL)
    {
      /* Allocate temporary uid and gid arrays at least
	 big enough to hold one effective ID.  */
      const unsigned int nuids = _hurd_nuids < 3 ? 3 : _hurd_nuids;
      const unsigned int ngids = _hurd_ngids < 3 ? 3 : _hurd_ngids;
      struct idlist *ruid = alloca (sizeof (uid_t) * _hurd_nuids);
      struct idlist *rgid = alloca (sizeof (gid_t) * _hurd_ngids);

      /* Copy all of our uids and gids to these arrays.  */
      memcpy (ruid, _hurd_uid, sizeof (uid_t) * nuids);
      memcpy (rgid, _hurd_gid, sizeof (gid_t) * ngids);

      /* Make the effective IDs be the real ones.  */
      ruid.ids[0] = ruid.rid;
      rgid.ids[0] = rgid.rid;

      /* Create a new auth port using these frobbed IDs.  */
      if (err = _HURD_PORT_USE (&_hurd_ports[INIT_PORT_AUTH],
				__auth_makeauth (port,
						 ruid, nuids,
						 rgid, ngids,
						 &_hurd_id.rid_auth)))
	goto lose;
    }

  crdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir);
  err = __io_reauthenticate (crdir);
  if (!err)
    {
      err = __auth_user_authenticate (_hurd_rid_auth, crdir, &rcrdir);
      __mach_port_deallocate (__mach_task_self (), crdir);
    }
  _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir, crdir);

  if (!err)
    {
      cwdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CWDIR], &dealloc_cwdir);
      err = __io_reauthenticate (cwdir);
      if (!err)
	{
	  err = __auth_user_authenticate (_hurd_rid_auth, cwdir, &rcwdir);
	  __mach_port_deallocate (__mach_task_self (), cwdir);
	}
      _hurd_port_free (cwdir, &dealloc_cwdir);
    }

  /* We are done with _hurd_rid_auth now.  */
  __mutex_unlock (&_hurd_idlock);

  if (err)
    return __hurd_fail (err);

  /* Now do a path lookup on FILE, using the crdir and cwdir
     reauthenticated with _hurd_rid_auth.  */

  flags = 0;
  if (type & R_OK)
    flags |= O_READ;
  if (type & W_OK)
    flags |= O_WRITE;
  if (type & X_OK)
    flags |= O_EXEC;

  err = __hurd_path_lookup (rcrdir, rcwdir, file, flags, 0, &io);
  __mach_port_deallocate (__mach_task_self (), rcrdir);
  __mach_port_deallocate (__mach_task_self (), rcwdir);
  if (err)
    return __hurd_fail (err);

  __mach_port_deallocate (__mach_task_self (), io);
  return 0;

 lose:
  __mutex_unlock (&_hurd_idlock);
  return __hurd_fail (err);
}
