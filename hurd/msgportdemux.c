/* Demux messages sent on the signal port.

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

#include <hurd.h>

const struct
  {
    size_t n;
    mach_msg_id_t id[0];
  } _hurd_sigport_ids;
const struct
  {
    size_t n;
    void (*routine[0]) (mach_msg_header_t *inp, mach_msg_header_t *outp);
  } _hurd_sigport_routines;

static boolean_t
_hurd_sigport_demux (mach_msg_header_t *inp,
		     mach_msg_header_t *outp)
{
  size_t i;

  if (_hurd_sigport_ids.n != _hurd_sigport_routines.n)
    __libc_fatal ("LIBRARY BUG: bogus sigport demux table");

  for (i = 0; i < _hurd_sigport_ids.n; ++i)
    if (inp->msgh_id == _hurd_sigport_ids.id[i])
      {
	(*_hurd_sigport_routines.routine[i]) (inp, outp);
	return 1;
      }

  {
    mig_reply_header_t *r = (mig_reply_header_t *) outp;
    r->RetCode = MIG_BAD_ID;
    return 0;
  }
}

/* This is the code that the signal thread runs.  */
void
_hurd_sigport_receive (void)
{
  while (1)
    (void) __mach_msg_server (_hurd_sigport_demux, __vm_page_size,
			      _hurd_sigport);
}
