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

/* Return the effective uid set.  */
int
getuids (int n, uid_t *uidset)
{
  int nuids;

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
  nuids = _hurd_id.nuids;
  if (uidset != NULL)
    {
      /* Copy into a temporary array before releasing the lock.  */
      uid_t uids[nuids];
      memcpy (uids, _hurd_id.uids, sizeof (uids));
      __mutex_unlock (&_hurd_idlock);
      /* Lock is released; now copy into user array, which might fault.  */
      memcpy (uidset, uids, sizeof (uids));
    }
  else
    __mutex_unlock (&_hurd_idlock);

  return nuids;
}
