/* Copyright (C) 2002-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <setjmp.h>
#include <stdlib.h>
#include "pthreadP.h"

/* Called by __syscall_cancel_arch or function above start the thread
   cancellation.  */
_Noreturn void
__syscall_do_cancel (void)
{
  struct pthread *self = THREAD_SELF;

  /* Disable thread cancellation to avoid cancellable entrypoints calling
     __syscall_do_cancel recursively.  We atomic load relaxed to check the
     state of cancelhandling, there is no particular ordering requirement
     between the syscall call and the other thread setting our cancelhandling
     with a atomic store acquire.

     POSIX Issue 7 notes that the cancellation occurs asynchronously on the
     target thread, that implies there is no ordering requirements.  It does
     not need a MO release store here.  */
  int oldval = atomic_load_relaxed (&self->cancelhandling);
  while (1)
    {
      int newval = oldval | CANCELSTATE_BITMASK;
      if (oldval == newval)
	break;
      if (atomic_compare_exchange_weak_acquire (&self->cancelhandling,
						&oldval, newval))
	break;
    }

  __do_cancel (PTHREAD_CANCELED);
}
