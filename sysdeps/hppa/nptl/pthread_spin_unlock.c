/* Copyright (C) 2005-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include "pthreadP.h"

int
pthread_spin_unlock (pthread_spinlock_t *lock)
{
  /* The LWS-CAS operation on hppa is a synthetic atomic operation
     that doesn't provide the type of coherency that we need. Therefore
     we force that coherency by using LWS-CAS again.  */
  atomic_exchange_rel (lock, 0);
  return 0;
}
