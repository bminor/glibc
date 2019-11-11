/* Set a small stack size, but still large enough for glibc's needs.
   Copyright (C) 2019 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <pthread.h>
#include <support/xthread.h>
#include <sys/param.h>

void
support_set_small_thread_stack_size (pthread_attr_t *attr)
{
  /* Some architectures have too small values for PTHREAD_STACK_MIN
     which cannot be used for creating threads.  Ensure that the stack
     size is at least 256 KiB.  */
  xpthread_attr_setstacksize (attr, MAX (256 * 1024, PTHREAD_STACK_MIN));
}
