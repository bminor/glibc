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

#include <mach.h>
#include <hurd/threadvar.h>

#define GETPORT \
  mach_port_t *portloc = \
    (mach_port_t *) __hurd_threadvar_location (_HURD_THREADVAR_MIG_REPLY)
#define reply_port (use_threadvar ? *portloc : global_reply_port)

static int use_threadvar;
static mach_port_t global_reply_port;

/* These functions are called by MiG-generated code.  */

/* Called by MiG to get a reply port.  */
mach_port_t
__mig_get_reply_port (void)
{
  GETPORT;

  if (reply_port == MACH_PORT_NULL)
    reply_port = __mach_reply_port ();

  return reply_port;
}

/* Called by MiG to deallocate the reply port.  */
void
__mig_dealloc_reply_port (void)
{
  mach_port_t port;

  GETPORT;

  port = reply_port;
  reply_port = MACH_PORT_NULL;	/* So the mod_refs RPC won't use it.  */
  __mach_port_mod_refs (__mach_task_self (), port,
			MACH_PORT_RIGHT_RECEIVE, -1);
}


/* Called at startup with CPROC == NULL.  When per-thread variables are set
   up, this is called again with CPROC a non-null value.  */
void
__mig_init (void *cproc)
{
  GETPORT;
  
  use_threadvar = cproc != 0;

  if (use_threadvar)
    {
      /* Recycle the reply port used before multithreading was enabled.  */
      *portloc = global_reply_port;
      global_reply_port = MACH_PORT_NULL;
    }
}

#include <gnu-stabs.h>

/* After fork, the child thread needs its own new reply port.  */

static void
mig_fork_child (void)
{
  GETPORT;
  *portloc = MACH_PORT_NULL;
}

text_set_element (_hurd_fork_child_hook, mig_fork_child);
