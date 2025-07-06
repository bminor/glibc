/* Libc stubs for pthread functions.  Hurd pthread version.
   Copyright (C) 2002-2025 Free Software Foundation, Inc.
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

#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <shlib-compat.h>
#include <pthread-functions.h>
#include <libc-lock.h>
#include <pt-internal.h>

/* Pointers to the libc functions.  */
struct pthread_functions __libc_pthread_functions attribute_hidden;
int __libc_pthread_functions_init attribute_hidden;


#define FORWARD2(name, rettype, decl, params, defaction) \
rettype									      \
name decl								      \
{									      \
  if (!__libc_pthread_functions_init)					      \
    defaction;								      \
									      \
  return PTHFCT_CALL (ptr_##name, params);				      \
}

/* Same as FORWARD2, only without return.  */
#define FORWARD_NORETURN(name, rettype, decl, params, defaction) \
rettype									      \
name decl								      \
{									      \
  if (!__libc_pthread_functions_init)					      \
    defaction;								      \
									      \
  PTHFCT_CALL (ptr_##name, params);					      \
}

#define FORWARD(name, decl, params, defretval) \
  FORWARD2 (name, int, decl, params, return defretval)

/* Use an alias to avoid warning, as pthread_exit is declared noreturn.  */
FORWARD_NORETURN (__pthread_exit, void, (void *retval), (retval),
		  exit (EXIT_SUCCESS))
strong_alias (__pthread_exit, pthread_exit);
