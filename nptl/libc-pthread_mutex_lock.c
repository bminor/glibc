/* Copyright (C) 2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef HAVE_ASM_SECONDARY_DIRECTIVE
# include "pthreadP.h"

/* Make sure that it is used only when libpthread is not used.  */
asm (".secondary __pthread_mutex_lock");
asm (".secondary pthread_mutex_lock");

int
__pthread_mutex_lock (pthread_mutex_t *mutex)
{
  return 0;
}

strong_alias (__pthread_mutex_lock, pthread_mutex_lock)
#endif
