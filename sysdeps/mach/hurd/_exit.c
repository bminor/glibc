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

#include <ansidecl.h>
#include <unistd.h>
#include <hurd.h>
#include <sysdep.h>
#include <sys/wait.h>

__NORETURN void
_hurd_exit (int status)
{
#ifdef notyet
  /* Give the proc server our exit status.  */
  _HURD_PORT_USE (&_hurd_ports[INIT_PORT_PROC], __proc_exit (port, status));
#endif

  /* Commit suicide.  */
  __task_terminate (__mach_task_self ());

  /* Perhaps the cached mach_task_self was bogus.  */
  __task_terminate ((__mach_task_self) ());
  
  /* This sucker really doesn't want to die.  */
  while (1)
    {
#ifdef LOSE
      LOSE;
#else
      volatile const int zero = 0, one = 1;
      volatile int lossage = one / zero;
#endif
    }
}

__NORETURN void
DEFUN(_exit, (status), int status)
{
  _hurd_exit (W_EXITCODE (status, 0));
}
