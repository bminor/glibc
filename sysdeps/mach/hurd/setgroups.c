/* Copyright (C) 1991 Free Software Foundation, Inc.
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
#include <errno.h>
#include <sys/types.h>
#include <grp.h>
#include <hurd.h>

/* Set the group set for the current user to GROUPS (N of them).  */
int
DEFUN(setgroups, (n, groups), size_t n AND CONST gid_t *groups)
{
  error_t err;
  auth_t newauth;

  if (n > sizeof (_hurd_id.groups) / sizeof (gid_t))
    {
      errno = EINVAL;
      return -1;
    }

  __mutex_lock (&_hurd_idlock);
  if (!_hurd_id_valid)
    {
      error_t err = __auth_getids (_hurd_auth, &_hurd_id);
      if (err)
	{
	  __mutex_unlock (&_hurd_idlock);
	  return __hurd_fail (err);
	}
      _hurd_id_valid = 1;
    }

  _hurd_id.ngroups = n;
  memcpy (_hurd_id.groups, groups, n * sizeof (gid_t));
  _hurd_id_valid = 0;
  err = __auth_makeauth (_hurd_auth, &_hurd_id, &newauth);
  __mutex_unlock (&_hurd_idlock);
  if (err)
    return __hurd_fail (err);

  return __setauth (newauth);
}
