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
#include <fcntl.h>

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
      if (_hurd_id.aux.nuids < 1 || _hurd_id.aux.ngids < 1)
	{
	  /* We do not have a real UID and GID.  Lose, lose, lose!  */
	  __mutex_unlock (&_hurd_id.lock);
	  return __hurd_fail (EGRATUITOUS);
	}

      /* Create a new auth port using our real UID and GID (the first
	 auxiliary UID and GID) as the only effective IDs.  */
      if (err = __USEPORT (AUTH,
			   __auth_makeauth (port,
					    NULL, 0, MACH_MSG_TYPE_COPY_SEND,
					    _hurd_id.aux.uids, 1,
					    _hurd_id.aux.gids, 1,
					    _hurd_id.aux.uids,
					    _hurd_id.aux.nuids,
					    _hurd_id.aux.gids,
					    _hurd_id.aux.ngids,
					    &_hurd_id.rid_auth)))
	goto lose;
    }

  crdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir);
  err = __io_reauthenticate (crdir, _hurd_pid);
  if (!err)
    {
      err = __auth_user_authenticate (_hurd_id.rid_auth,
				      crdir, _hurd_pid, &rcrdir);
      __mach_port_deallocate (__mach_task_self (), crdir);
    }
  _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir, crdir);

  if (!err)
    {
      cwdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CWDIR], &dealloc_cwdir);
      err = __io_reauthenticate (cwdir, _hurd_pid);
      if (!err)
	{
	  err = __auth_user_authenticate (_hurd_id.rid_auth,
					  cwdir, _hurd_pid, &rcwdir);
	  __mach_port_deallocate (__mach_task_self (), cwdir);
	}
      _hurd_port_free (&_hurd_ports[INIT_PORT_CWDIR], &dealloc_cwdir, cwdir);
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
