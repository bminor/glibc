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

#ifndef _LOCK_INTERN_H
#define	_LOCK_INTERN_H

#include <machine-lock.h>

#ifndef _EXTERN_INLINE
#define _EXTERN_INLINE extern __inline
#endif


/* Initialize LOCK.  */

_EXTERN_INLINE void
__spin_lock_init (__spin_lock_t *__lock)
{
  *__lock = __SPIN_LOCK_INITIALIZER;
}


/* Lock LOCK, blocking if we can't get it.  */
extern void __spin_lock_solid (__spin_lock_t *__lock);

/* Lock the spin lock LOCK.  */

_EXTERN_INLINE void
__spin_lock (__spin_lock_t *__lock)
{
  if (! __spin_try_lock (__lock))
    __spin_lock_solid (__lock);
}


#define __mutex_lock(lockaddr) __spin_lock (&(lockaddr)->lock) /* XXX */
#define __mutex_unlock(lockaddr) __spin_unlock (&(lockaddr)->lock) /* XXX */


#endif /* lock-intern.h */
