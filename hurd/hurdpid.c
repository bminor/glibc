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
#include <gnu-stabs.h>

pid_t _hurd_pid, _hurd_ppid, _hurd_pgrp;
int _hurd_orphaned;

static void
init_pids (void)
{
  int dealloc;
  process_t proc = _hurd_port_get (&_hurd_proc, &dealloc);

  __proc_getpids (proc, &_hurd_pid, &_hurd_ppid, &_hurd_orphaned);
  __proc_getpgrp (proc, _hurd_pid, &_hurd_pgrp);

  _hurd_port_free (proc, &dealloc);
}

text_set_element (__libc_subinit, init_pids);

static error_t
proc_newids (sigthread_t me,
	     pid_t ppid, pid_t pgrp, int orphaned)
{
  _hurd_ppid = ppid;
  _hurd_pgrp = pgrp;
  _hurd_orphaned = orphaned;
  return POSIX_SUCCESS;
}

#include "misc/proc_newids.c"

asm (".stabs \"__hurd_sigport_ids\",23,0,0,23002"); /* XXX */
text_set_element (_hurd_sigport_routines, _Xproc_newids);
