/* Copyright (C) 1992 Free Software Foundation, Inc.
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

#include <mach.h>
#include <mach/kernel_boot.h>

kern_return_t
__mach_get_priv_ports (host_priv_t *host_priv_ptr, device_t *device_master_ptr)
{
  kern_return_t err;
  mach_port_t bootstrap;

  if (err = task_get_bootstrap_port (mach_task_self (), &bootstrap))
    return err;
  err = __kernel_boot_get_priv_ports (bootstrap,
				      host_priv_ptr, dev_master_ptr);
  mach_port_deallocate (mach_task_self (), bootstrap);
  return err;
}
