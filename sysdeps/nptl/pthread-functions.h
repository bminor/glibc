/* Copyright (C) 2003-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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
   <https://www.gnu.org/licenses/>.  */

#ifndef _PTHREAD_FUNCTIONS_H
#define _PTHREAD_FUNCTIONS_H	1

#include <pthread.h>
#include <setjmp.h>
#include <internaltypes.h>
#include <sysdep.h>

struct xid_command;

/* Data type shared with libc.  The libc uses it to pass on calls to
   the thread functions.  */
struct pthread_functions
{
  int (*ptr__nptl_setxid) (struct xid_command *);
};

/* Variable in libc.so.  */
extern struct pthread_functions __libc_pthread_functions attribute_hidden;
extern int __libc_pthread_functions_init attribute_hidden;

#ifdef PTR_DEMANGLE
# define PTHFCT_CALL(fct, params) \
  ({ __typeof (__libc_pthread_functions.fct) __p;			      \
     __p = __libc_pthread_functions.fct;				      \
     PTR_DEMANGLE (__p);						      \
     __p params; })
#else
# define PTHFCT_CALL(fct, params) \
  __libc_pthread_functions.fct params
#endif

#endif	/* pthread-functions.h */
