/* Single-thread optimization definitions.  Linux version.
   Copyright (C) 2017-2020 Free Software Foundation, Inc.

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

#ifndef _SYSDEP_CANCEL_H
#define _SYSDEP_CANCEL_H

#include <sysdep.h>
#include <tls.h>
#include <errno.h>

/* The two functions are in libc.so and not exported.  */
extern int __libc_enable_asynccancel (void) attribute_hidden;
extern void __libc_disable_asynccancel (int oldtype) attribute_hidden;

/* The two functions are in librt.so and not exported.  */
extern int __librt_enable_asynccancel (void) attribute_hidden;
extern void __librt_disable_asynccancel (int oldtype) attribute_hidden;

/* The two functions are in libpthread.so and not exported.  */
extern int __pthread_enable_asynccancel (void) attribute_hidden;
extern void __pthread_disable_asynccancel (int oldtype) attribute_hidden;

/* Set cancellation mode to asynchronous.  */
#define CANCEL_ASYNC() \
  __pthread_enable_asynccancel ()
/* Reset to previous cancellation mode.  */
#define CANCEL_RESET(oldtype) \
  __pthread_disable_asynccancel (oldtype)

#if IS_IN (libc)
/* Same as CANCEL_ASYNC, but for use in libc.so.  */
# define LIBC_CANCEL_ASYNC() \
  __libc_enable_asynccancel ()
/* Same as CANCEL_RESET, but for use in libc.so.  */
# define LIBC_CANCEL_RESET(oldtype) \
  __libc_disable_asynccancel (oldtype)
#elif IS_IN (libpthread)
# define LIBC_CANCEL_ASYNC() CANCEL_ASYNC ()
# define LIBC_CANCEL_RESET(val) CANCEL_RESET (val)
#elif IS_IN (librt)
# define LIBC_CANCEL_ASYNC() \
  __librt_enable_asynccancel ()
# define LIBC_CANCEL_RESET(val) \
  __librt_disable_asynccancel (val)
#else
# define LIBC_CANCEL_ASYNC()	0 /* Just a dummy value.  */
# define LIBC_CANCEL_RESET(val)	((void)(val)) /* Nothing, but evaluate it.  */
#endif


#endif
