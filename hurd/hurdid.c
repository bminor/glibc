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

#include <hurd.h>
#include <gnu-stabs.h>

struct mutex _hurd_idlock;
int _hurd_id_valid;
struct idblock _hurd_id;

static void
init_id (void)
{
  __mutex_init (&_hurd_idlock);
  _hurd_id_valid = 0;
}

text_set_element (__libc_subinit, init_id);

/* Check that _hurd_uid and _hurd_gid are valid and update them if not.
   Expects _hurd_idlock to be held and does not release it.  */

error_t
_hurd_check_ids (void)
{
  if (! _hurd_id_valid)
    {
      error_t err;

      if (_hurd_uid)
	{
	  __vm_deallocate (__mach_task_self (), _hurd_uid);
	  _hurd_uid = NULL;
	}
      if (_hurd_gid)
	{
	  __vm_deallocate (__mach_task_self (), _hurd_gid);
	  _hurd_gid = NULL;
	}
      if (_hurd_rid_auth)
	{
	  __mach_port_deallocate (__mach_task_self (), _hurd_rid_auth);
	  _hurd_rid_auth = MACH_PORT_NULL;
	}

      if (err = _HURD_PORT_USE (&_hurd_ports[INIT_PORT_AUTH],
				__auth_getids (port,
					       &_hurd_uid, &_hurd_nuids,
					       &_hurd_gid, &_hurd_ngids)))
	return err;

      _hurd_id_valid = 1;
    }

  return 0;
}
