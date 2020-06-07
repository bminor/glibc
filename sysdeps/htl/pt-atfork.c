/* Register fork handlers.  Generic version.
   Copyright (C) 2002-2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library;  if not, see
   <https://www.gnu.org/licenses/>.  */

#include <pthread.h>
#include <pt-internal.h>
#include <fork.h>
#include <dso_handle.h>

/* Hide the symbol so that no definition but the one locally in the
   executable or DSO is used.  */
int
#ifndef __pthread_atfork
/* Don't mark the compatibility function as hidden.  */
attribute_hidden
#endif
__pthread_atfork (void (*prepare) (void),
		void (*parent) (void),
		void (*child) (void))
{
  return __register_atfork (prepare, parent, child, __dso_handle);
}
#ifndef __pthread_atfork
extern int pthread_atfork (void (*prepare) (void), void (*parent) (void),
                           void (*child) (void)) attribute_hidden;
weak_alias (__pthread_atfork, pthread_atfork)
#endif
