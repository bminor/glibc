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

#include <ansidecl.h>
#include <unistd.h>
#include <hurd.h>

void
DEFUN(_exit, (status), int status)
{
  /* Does not return (when applied to the calling task).  */
  extern volatile void __task_terminate (task_t);

  struct _hurd_sigstate *ss = _hurd_thread_sigstate (__mach_thread_self ());

  __proc_exit (_hurd_proc, status);

  if (ss->vforked)
    longjmp (ss->vfork_saved.continuation, 1);

  __task_terminate (__mach_task_self ());
}
