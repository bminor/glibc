/* Copyright (C) 1991, 1993, 1994 Free Software Foundation, Inc.
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
#include <sys/resource.h>
#include <errno.h>
#include <hurd.h>
#include <hurd/fd.h>

extern int _hurd_core_limit, _hurd_dtable_rlimit; /* XXX */


/* Put the soft and hard limits for RESOURCE in *RLIMITS.
   Returns 0 if successful, -1 if not (and sets errno).  */
int
DEFUN(getrlimit, (resource, rlimits),
      enum __rlimit_resource resource AND struct rlimit *rlimits)
{
  if (rlimits == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  switch (resource)
    {
    default:
      errno = EINVAL;
      return -1;

    case RLIMIT_CORE:
      rlimits->rlim_cur = _hurd_core_limit;
      rlimits->rlim_max = RLIM_INFINITY;
      break;

    case RLIMIT_DATA:
      rlimits->rlim_cur = _hurd_data_end;
      rlimits->rlim_max = RLIM_INFINITY;
      break;

    case RLIMIT_OFILE:
      {
	int lim;
	HURD_CRITICAL_BEGIN;
	__mutex_lock (&_hurd_dtable_lock);
	lim = _hurd_dtable_rlimit;
	__mutex_unlock (&_hurd_dtable_lock);
	HURD_CRITICAL_END;
	rlimits->rlim_cur = lim;
	rlimits->rlim_max = RLIM_INFINITY;
	break;
      }

    case RLIMIT_CPU:
    case RLIMIT_FSIZE:
    case RLIMIT_STACK:
    case RLIMIT_RSS:
      errno = ENOSYS;
      return -1;
    }

  return 0;
}
