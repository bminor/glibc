/* Copyright (C) 1991, 1994 Free Software Foundation, Inc.
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
#include <mach.h>
#include <thread_state.h>

#define	STACK_SIZE	(4 * __vm_page_size) /* ? XXX */

/* Give THREAD a stack and set it to run at PC when resumed.  */
error_t
_hurd_setup_thread (task_t task, thread_t thread, void *pc)
{
  error_t error;
  struct hurd_thread_state ts;
  size_t tssize = HURD_THREAD_STATE_COUNT;
  vm_address_t stack;

  if (error = __vm_allocate (task, &stack, STACK_SIZE, 1))
    return error;

  memset (&ts, 0, sizeof (ts));
  ts.PC = (int) pc;
  ts.SP = stack + STACK_SIZE;

  return __thread_set_state (thread, HURD_THREAD_STATE_FLAVOR,
			     (int *) &ts, tssize);
}
