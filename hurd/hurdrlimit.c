/* Resource limits.
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

#include <hurd.h>
#include <cthreads.h>
#include <hurd/resource.h>
#include <gnu-stabs.h>

struct rlimit _hurd_rlimits[RLIM_NLIMITS];
struct mutex _hurd_rlimit_lock;

static void
init_rlimit (void)
{
  int i;

  for (i = 0; i < RLIM_NLIMITS; ++i)
    {
      if (_hurd_rlimits[i].rlim_max == 0)
	_hurd_rlimits[i].rlim_max = RLIM_INFINITY;
      _hurd_rlimits[i].rlim_cur = _hurd_rlimits[i].rlim_max;
    }

  __mutex_init (&_hurd_rlimit_lock);

  (void) &init_rlimit;
}
text_set_element (_hurd_subinit, init_rlimit);
