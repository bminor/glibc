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

#include <hurd.h>

static inline mach_port_t
get (const size_t idx)
{
  mach_port_t result;
  error_t err = _HURD_PORT_USE (&_hurd_ports[idx],
				__mach_port_mod_refs (__mach_task_self (),
						      (result = port),
						      MACH_PORT_RIGHT_SEND,
						      1));
  if (err)
    {
      errno = err;
      return MACH_PORT_NULL;
    }
  else
    return result;
}
#define	GET(type, what, idx) \
  type get##what (void) { return get (INIT_PORT_##idx); }

static inline int
set (const size_t idx, mach_port_t new)
{
  error_t err;
  if (err = __mach_port_mod_refs (__mach_task_self (), new,
				  MACH_PORT_RIGHT_SEND, 1))
    {
      errno = EINVAL;
      return -1;
    }
  _hurd_port_set (&_hurd_ports[idx], new);
  return 0;
}
#define SET(type, what, idx) \
  int set##what (type new) { return set (INIT_PORT_##idx, new); }

#define	GETSET(type, what, idx) \
  GET (type, what, idx) SET (type, what, idx)

GETSET (process_t, proc, PROC)
GETSET (file_t, cwdir, CWDIR)
GETSET (file_t, crdir, CRDIR)

/* setauth is nontrivial; see __setauth.c.  */
GET (auth_t, auth, AUTH)
