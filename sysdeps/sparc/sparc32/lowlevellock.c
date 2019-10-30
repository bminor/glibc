/* low level locking for pthread library.  SPARC version.
   Copyright (C) 2003-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <sys/time.h>
#include <time.h>


void
__lll_lock_wait_private (int *futex)
{
  do
    {
      int oldval = atomic_compare_and_exchange_val_24_acq (futex, 2, 1);
      if (oldval != 0)
	lll_futex_wait (futex, 2, LLL_PRIVATE);
    }
  while (atomic_compare_and_exchange_val_24_acq (futex, 2, 0) != 0);
}


/* These functions don't get included in libc.so  */
#if IS_IN (libpthread)
void
__lll_lock_wait (int *futex, int private)
{
  do
    {
      int oldval = atomic_compare_and_exchange_val_24_acq (futex, 2, 1);
      if (oldval != 0)
	lll_futex_wait (futex, 2, private);
    }
  while (atomic_compare_and_exchange_val_24_acq (futex, 2, 0) != 0);
}
#endif
