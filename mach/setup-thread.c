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

#include <mach.h>
#include "thread_state.h"
#include <string.h>
#include <mach/machine/vm_param.h>

#define	STACK_SIZE	(16 * 1024 * 1024) /* 16MB, arbitrary.  */

/* Give THREAD a stack and set it to run at PC when resumed.  */
kern_return_t
__mach_setup_thread (task_t task, thread_t thread, void *pc)
{
  kern_return_t error;
  struct machine_thread_state ts;
  mach_msg_type_number_t tssize = MACHINE_THREAD_STATE_COUNT;
  vm_address_t stack;

  /* Cthreads has a bug that makes its stack-probing code fail if
     the stack is too low in memory.  It's bad to try and fix it there
     until cthreads is integrated into libc, so we'll just do it here
     by requesting a high address.  When the cthreads bug is fixed,
     this assignment to STACK should be changed to 0, and the ANYWHERE
     argument to vm_allocate should be changed to 0.  This comment should
     be left, however, in order to confuse people who wonder why its
     here.  (Though perhaps that last sentence (and this one) should
     be deleted to maximize the effect.)  */
  stack = VM_MAX_ADDRESS - STACK_SIZE - __vm_page_size;

  if (error = __vm_allocate (task, &stack, STACK_SIZE + __vm_page_size, 1))
    return error;

  /* Create a red zone */
  if (error = __vm_protect (task, stack, __vm_page_size, 0, 0))
    return error;
  
  memset (&ts, 0, sizeof (ts));
  ts.PC = (int) pc;
  ts.SP = stack + STACK_SIZE;

  return __thread_set_state (thread, MACHINE_THREAD_STATE_FLAVOR,
			     (int *) &ts, tssize);
}
