/* Declaration of libc stubs for pthread functions.  Hurd version.
   Copyright (C) 2003-2025 Free Software Foundation, Inc.
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

#ifndef _PTHREAD_FUNCTIONS_H
#define _PTHREAD_FUNCTIONS_H	1

#include <pthread.h>

void __pthread_exit (void *) __attribute__ ((__noreturn__));
struct __pthread_cancelation_handler **__pthread_get_cleanup_stack (void);
int __pthread_once (pthread_once_t *, void (*) (void));
int __pthread_rwlock_rdlock (pthread_rwlock_t *);
int __pthread_rwlock_wrlock (pthread_rwlock_t *);
int __pthread_rwlock_unlock (pthread_rwlock_t *);
int __pthread_key_create (pthread_key_t *, void (*) (void *));
void *__pthread_getspecific (pthread_key_t);
int __pthread_setspecific (pthread_key_t, const void *);

void _cthreads_flockfile (FILE *);
void _cthreads_funlockfile (FILE *);
int _cthreads_ftrylockfile (FILE *);

/* Data type shared with libc.  The libc uses it to pass on calls to
   the thread functions.  Wine pokes directly into this structure,
   so if possible avoid breaking it and append new hooks to the end.  */
struct pthread_functions
{
  void (*ptr___pthread_exit) (void *) __attribute__ ((__noreturn__));
  struct __pthread_cancelation_handler **(*ptr___pthread_get_cleanup_stack) (void);
  int (*ptr_pthread_once) (pthread_once_t *, void (*) (void));
  int (*ptr_pthread_rwlock_rdlock) (pthread_rwlock_t *);
  int (*ptr_pthread_rwlock_wrlock) (pthread_rwlock_t *);
  int (*ptr_pthread_rwlock_unlock) (pthread_rwlock_t *);
  int (*ptr___pthread_key_create) (pthread_key_t *, void (*) (void *));
  void *(*ptr___pthread_getspecific) (pthread_key_t);
  int (*ptr___pthread_setspecific) (pthread_key_t, const void *);
  void (*ptr__IO_flockfile) (FILE *);
  void (*ptr__IO_funlockfile) (FILE *);
  int (*ptr__IO_ftrylockfile) (FILE *);
};

/* Variable in libc.so.  */
extern struct pthread_functions __libc_pthread_functions attribute_hidden;
extern int __libc_pthread_functions_init attribute_hidden;

void __libc_pthread_init (const struct pthread_functions *functions);

#define PTHFCT_CALL(fct, params) \
    __libc_pthread_functions.fct params

#endif	/* pthread-functions.h */
