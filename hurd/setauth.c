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

#include <hurd.h>

static inline void
reauth_io (io_t *server)
{
  if (*server != MACH_PORT_NULL &&
      __io_reauthenticate (*server) == POSIX_SUCCESS)
    {
      mach_port_t new;
      if (__auth_user_authenticate (_hurd_auth,
				    *server,
				    &new) == POSIX_SUCCESS)
	{
	  __mach_port_deallocate (__mach_task_self (),
				  *server);
	  *server = new;
	}
    }
}


/* Set the auth port to NEW, and reauthenticate
   everything used by the library.  */
int
__setauth (auth_t new)
{
  error_t err;
  auth_t old;
  int d;
  mach_port_t ignore;

  if (__mach_port_mod_refs (__mach_task_self (), new,
			    MACH_PORT_RIGHT_SEND, 1))
    {
      errno = EINVAL;
      return -1;
    }

  __mutex_lock (&_hurd_idlock);
  __mutex_lock (&_hurd_dtable_lock);
  __mutex_lock (&_hurd_lock);
  old = _hurd_auth;
  _hurd_auth = new;
  _hurd_id_valid = 0;
  __mutex_unlock (&_hurd_idlock);
  __mach_port_deallocate (__mach_task_self (), old);
  __proc_reauthenticate (_hurd_proc);
  __auth_user_authenticate (_hurd_auth, _hurd_proc, &ignore);
  if (ignore != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), ignore);
  if (_hurd_dtable.d == NULL)
    for (d = 0; d < _hurd_init_dtablesize; ++d)
      reauth_io (&_hurd_init_dtable[d]);
  else
    for (d = 0; d < _hurd_dtable.size; ++d)
      reauth_io (&_hurd_dtable.d[d]);
  __mutex_unlock (&_hurd_dtable_lock);

  return 0;
}
