/* Initialize pthreads library.
   Copyright (C) 2000-2025 Free Software Foundation, Inc.
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

#include <assert.h>
#include <string.h>

#include <pt-internal.h>
#include <set-hooks.h>

#include <pthread.h>
#include <pthread-functions.h>

#if IS_IN (libpthread)
static const struct pthread_functions pthread_functions = {
  .ptr___pthread_exit = __pthread_exit,
  .ptr__IO_flockfile = _cthreads_flockfile,
  .ptr__IO_funlockfile = _cthreads_funlockfile,
  .ptr__IO_ftrylockfile = _cthreads_ftrylockfile,
};
#endif /* IS_IN (libpthread) */

/* Initialize the pthreads library.  */
void
___pthread_init (void)
{
#if IS_IN (libpthread)
  __libc_pthread_init (&pthread_functions);
#endif
}
