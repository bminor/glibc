/* Copyright (C) 1994 Free Software Foundation, Inc.
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

static error_t
setbootstrap (mach_port_t newport)
{
  error_t err;
  err = __task_set_special_port (__mach_task_self (),
				 TASK_BOOTSTRAP_PORT,
				 newport);
  if (err == 0)
    /* Consume the reference for NEWPORT when successful.  */
    __mach_port_deallocate (__mach_task_self (), newport);
  return err;
}

error_t (*_hurd_ports_setters[INIT_PORT_MAX]) (mach_port_t newport) =
  {
    [INIT_PORT_BOOTSTRAP] = setbootstrap,
    [INIT_PORT_AUTH] = _hurd_setauth,
    [INIT_PORT_PROC] = _hurd_setproc,
  };


error_t
_hurd_ports_set (int which, mach_port_t newport)
{
  error_t err;
  if (which < 0 || which >= _hurd_nports)
    return EINVAL;
  if (err = __mach_port_mod_refs (__mach_task_self (), new,
				  MACH_PORT_RIGHT_SEND, 1))
    return err;
  if (which >= INIT_PORT_MAX || _hurd_ports_setters[which] == NULL)
    {
      _hurd_port_set (&_hurd_ports[which], newport);
      return 0;
    }
  return (*_hurd_ports_setters[which]) (newport);
}
