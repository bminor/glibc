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

#include <hurd.h>

struct _hurd_port _hurd_auth;

error_t
__add_auth (sigthread_t me,
	    auth_t addauth)
{
  error_t err;
  auth_t newauth;

  if (err = _HURD_PORT_USE (&_hurd_auth,
			    __auth_combine (port, addauth, &newauth)))
    return err;

  /* XXX clobbers errno. Need per-thread errno. */
  err = __setauth (newauth);
  __mach_port_deallocate (__mach_task_self (), newauth);
  if (err)
    return errno;

  __mach_port_deallocate (__mach_task_self (), addauth);
  return POSIX_SUCCESS;
}

error_t
__del_auth (sigthread_t me, task_t task,
	    uid_t *uids, size_t nuids,
	    gid_t *gids, size_t ngids)
{
  error_t err;
  auth_t newauth;
  size_t i, j;

  if (task != __mach_task_self ())
    return EPERM;

  __mutex_lock (&_hurd_idlock);
  if (!_hurd_id_valid)
    {
      error_t err = _HURD_PORT_USE (&_hurd_auth,
				    __auth_getids (port, &_hurd_id));
      if (err)
	{
	  __mutex_unlock (&_hurd_idlock);
	  return err;
	}
      _hurd_id_valid = 1;
    }

  while (nuids-- > 0)
    {
      const uid_t uid = *uids++;
      for (i = 0; i < _hurd_id.nuids; ++i)
	if (_hurd_id.uidset[i] == uid)
	  /* Move the last uid into this slot, and decrease the
	     number of uids so the last slot is no longer used.  */
	  _hurd_id.uidset[i] = _hurd_id.uidset[--_hurd_id.nuids];
    }
  while (ngids-- > 0)
    {
      const gid_t gid = *gids++;
      for (i = 0; i < _hurd_id.ngroups; ++i)
	if (_hurd_id.gidset[i] == gid)
	  /* Move the last gid into this slot, and decrease the
	     number of gids so the last slot is no longer used.  */
	  _hurd_id.gidset[i] = _hurd_id.gidset[--_hurd_id.ngroups];
    }

  err = _HURD_PORT_USE (&_hurd_auth,
			__auth_makeauth (port, &_hurd_id, &newauth));
  _hurd_id_valid = !err;
  __mutex_unlock (&_hurd_idlock);

  if (err)
    return err;
  err = __setauth (newauth);	/* XXX clobbers errno */
  __mach_port_deallocate (__mach_task_self (), newauth);
  if (err)
    return errno;
  return POSIX_SUCCESS;
}
