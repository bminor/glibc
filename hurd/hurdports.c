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

#define GET(type, what)							      \
type get##what (void)							      \
{									      \
  __mach_port_mod_refs (__mach_task_self (), _hurd_##what,		      \
			MACH_PORT_RIGHT_SEND, 1);			      \
  return _hurd_##what;							      \
}

#define SET(lock, type, what)						      \
int									      \
set##what (type new)							      \
{									      \
  error_t err;								      \
  type old;								      \
  if (err = __mach_port_mod_refs (__mach_task_self (), new,		      \
				  MACH_PORT_RIGHT_SEND, 1))		      \
    {									      \
      errno = EINVAL;							      \
      return -1;							      \
    }									      \
  __mutex_lock (&_hurd_##lock);						      \
  old = _hurd_##what;							      \
  _hurd_##what = new;							      \
  __mutex_unlock (&_hurd_##lock);					      \
  __mach_port_deallocate (__mach_task_self (), old);			      \
  return 0;								      \
}

#define	GETSET(lock, type, what)	GET (type, what) SET (lock, type, what)

GETSET (lock, process_t, proc)
GETSET (lock, file_t, ccdir)
GETSET (lock, file_t, cwdir)
GETSET (lock, file_t, crdir)
GET (lock, auth_t, auth)

#define	getsigthread	siggetthread
#define	setsigthread	sigsetthread
GETSET (siglock, thread_t, sigthread)
