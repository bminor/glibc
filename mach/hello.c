/* "Hello world" program for GNU C Library on bare Mach 3.0.  */

Copyright (C) 1991 Free Software Foundation, Inc.
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
#include <mach.h>
#include <mach/device.h>
#include <errno.h>

extern FILE *mach_open_devstream (device_t dev);

int
main (int argc, char **argv, char **envp)
{
  error_t err;
  mach_port_t device, consdev;
  FILE *consf;

  device = pid2task (-2);
  err = device_open (device, D_WRITE, "console", consdev);
  mach_port_deallocate (mach_task_self (), device);
  if (err)
    exit (err);

  consf = mach_open_devstream (consdev);
  if (consf == NULL)
    exit (errno);

  fputs ("Hello, world!\n", consf);
  exit (0);
}
