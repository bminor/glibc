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

pid_t
__task2pid (task_t task)
{
  error_t err;
  pid_t pid;
  __mutex_lock (&_hurd_lock);
  err = __proc_task2pid (_hurd_proc, task, &pid);
  __mutex_unlock (&_hurd_lock);
  if (err)
    {
      errno = err;
      return (pid_t) -1
    }
  return pid;
}
