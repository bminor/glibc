/* Resource limits for the Hurd.
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

#ifndef _HURD_RESOURCE_H
#define _HURD_RESOURCE_H

#include <sys/types.h>
#include <sys/resource.h>

/* This array contains the current resource limits for the process.  */
extern struct rlimit _hurd_rlimits[RLIM_NLIMITS];
extern struct mutex _hurd_rlimit_lock; /* Locks _hurd_rlimits.  */


/* Helper function for getpriority and setpriority.
   Maps FUNCTION over all the processes specified by WHICH and WHO.
   Returns FUNCTION's result the first time it returns nonzero.
   If FUNCTION never returns nonzero, this returns zero.  */
extern error_t _hurd_priority_which_map (enum __priority_which which, int who,
					 error_t (*function) (pid_t));

#endif
