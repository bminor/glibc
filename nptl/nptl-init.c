/* Copyright (C) 2002-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <pthreadP.h>
#include <atomic.h>
#include <ldsodefs.h>
#include <tls.h>
#include <list.h>
#include <version.h>
#include <shlib-compat.h>
#include <lowlevellock.h>
#include <futex-internal.h>
#include <kernel-features.h>
#include <libc-pointer-arith.h>
#include <pthread_mutex_conf.h>
#include <nptl-stack.h>

/* Version of the library, used in libthread_db to detect mismatches.  */
static const char nptl_version[] __attribute_used__ = VERSION;

void
__pthread_initialize_minimal_internal (void)
{
}
strong_alias (__pthread_initialize_minimal_internal,
	      __pthread_initialize_minimal)


/* This function is internal (it has a GLIBC_PRIVATE) version, but it
   is widely used (either via weak symbol, or dlsym) to obtain the
   __static_tls_size value.  This value is then used to adjust the
   value of the stack size attribute, so that applications receive the
   full requested stack size, not diminished by the TCB and static TLS
   allocation on the stack.  Once the TCB is separately allocated,
   this function should be removed or renamed (if it is still
   necessary at that point).  */
size_t
__pthread_get_minstack (const pthread_attr_t *attr)
{
  return (GLRO(dl_pagesize) + __nptl_tls_static_size_for_stack ()
	  + PTHREAD_STACK_MIN);
}
