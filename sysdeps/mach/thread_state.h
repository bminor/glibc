/* Generic definitions for dealing with Mach thread states.
Copyright (C) 1994 Free Software Foundation, Inc.
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


/* Everything else is called `thread_state', but CMU's header file is
   called `thread_status'.  Oh boy.  */
#include <mach/thread_status.h>

/* These functions are of use in machine-dependent signal trampoline
   implementations.  */

static inline int
machine_get_state (thread_t thread, struct machine_thread_all_state *state,
		   int flavor, void *stateptr, void *scpptr, size_t size)
{
  if (state->set & (1 << flavor))
    {
      /* Copy the saved state.  */
      memcpy (scpptr, stateptr, size);
      return 1;
    }
  else
    {
      /* Noone asked about this flavor of state before; fetch the state
	 directly from the kernel into the sigcontext.  */
      unsigned int got;
      return (! __thread_get_state (thread, flavor, scpptr, &got)
	      && got == (size / sizeof (int)));
    }
}

static inline int
machine_get_basic_state (thread_t thread,
			 struct machine_thread_all_state *state)
{
  unsigned int count;

  if (state->set & (1 << flavor))
    return 1;

  if (__thread_get_state (thread, MACHINE_THREAD_STATE_FLAVOR,
			  (int *) &state->basic, &count) != KERN_SUCCESS ||
      count != MACHINE_THREAD_STATE_COUNT)
    /* What kind of thread?? */
    return 0;			/* XXX */

  state->set |= 1 << MACHINE_THREAD_STATE_FLAVOR;
  return 1;
}
