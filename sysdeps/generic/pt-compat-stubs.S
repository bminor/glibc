/* ABI compatibility stubs for libpthread.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

/* The functions defined by this file used to have two different
   definitions, one in libc.so and one in libpthread.so.  Nowadays,
   only the version in libc is necessary, but libpthread must continue
   to expose these symbols for compatibility's sake.  The stubs just
   jump to the definition in libc.

   This file is written in heavily macro-ized assembly language
   because one of the stubs that it needs to define is vfork, and the
   implementation of vfork must not touch the stack.  Having done the
   work to handle that, we may as well reuse the mechanism for all of
   the stubs.  */

#include <shlib-compat.h>
#include <sysdep.h>

/* We need __OFF_T_MATCHES_OFF64_T, but sys/types.h cannot be included
   from assembly language.  */
#define _BITS_TYPES_H 1
#include <bits/typesizes.h>

        compat_text_section

#ifndef SIBCALL_ENTRY
# define SIBCALL_ENTRY(name) ENTRY(name)
#endif
#ifndef LONGJMP_2_0_DEST
# define LONGJMP_2_0_DEST __libc_longjmp
#endif
#ifndef LONGJMP_2_0_UNTIL_VER
# define LONGJMP_2_0_UNTIL_VER GLIBC_2_22
#endif

#define define_stub(name)						      \
  define_stub_1(__pstub_##name, __libc_##name)
#define define_stub_with_dest(name, dest)				      \
  define_stub_1(__pstub_##name, dest)
#define define_stub_1(pstub_name, libc_name)				      \
  SIBCALL_ENTRY(pstub_name)   ASM_LINE_SEP				      \
  SIBCALL(libc_name)  ASM_LINE_SEP					      \
  END(pstub_name)

#define compat_stub(base, sym, ver)					      \
  compat_stub_1(base, sym, ver, __COUNTER__)
#define compat_stub_1(base, sym, ver, tag)				      \
  compat_stub_2(base, sym, ver, tag)
#define compat_stub_2(base, sym, ver, tag)				      \
  compat_stub_3(__pstub_##base, __pstub_##base##_##tag, sym, ver)
#define compat_stub_3(base, nonce, sym, ver)				      \
  weak_alias(base, nonce) ASM_LINE_SEP					      \
  compat_symbol(libpthread, nonce, sym, ver)

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_20) \
 || SHLIB_COMPAT (libpthread, GLIBC_2_1_2, GLIBC_2_20)

        define_stub(vfork)
# if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_20)
        compat_stub(vfork, vfork, GLIBC_2_0)
# endif
# if SHLIB_COMPAT (libpthread, GLIBC_2_1_2, GLIBC_2_20)
        compat_stub(vfork, __vfork, GLIBC_2_1_2)
# endif
#endif

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_22)

        define_stub(fork)
        compat_stub(fork, fork, GLIBC_2_0)
        compat_stub(fork, __fork, GLIBC_2_0)

        define_stub(system)
        compat_stub(system, system, GLIBC_2_0)

#endif

/* Some architectures need special treatment for longjmp.  */
#if SHLIB_COMPAT (libpthread, GLIBC_2_0, LONGJMP_2_0_UNTIL_VER)

        define_stub_with_dest(longjmp, LONGJMP_2_0_DEST)
        compat_stub(longjmp, longjmp, GLIBC_2_0)
        compat_stub(longjmp, siglongjmp, GLIBC_2_0)

#endif

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_28)

        define_stub(accept)
        compat_stub(accept, accept, GLIBC_2_0)

        define_stub(close)
        compat_stub(close, close, GLIBC_2_0)
        compat_stub(close, __close, GLIBC_2_0)

        define_stub(connect)
        compat_stub(connect, connect, GLIBC_2_0)
        compat_stub(connect, __connect, GLIBC_2_0)

        define_stub(fcntl)
        compat_stub(fcntl, fcntl, GLIBC_2_0)
        compat_stub(fcntl, __fcntl, GLIBC_2_0)

        define_stub(fsync)
        compat_stub(fsync, fsync, GLIBC_2_0)

        define_stub(msync)
        compat_stub(msync, msync, GLIBC_2_0)

        define_stub(nanosleep)
        compat_stub(nanosleep, nanosleep, GLIBC_2_0)
#if SHLIB_COMPAT (libpthread, GLIBC_2_2_6, GLIBC_2_28)
        compat_stub(nanosleep, __nanosleep, GLIBC_2_2_6)
#endif

        define_stub(pause)
        compat_stub(pause, pause, GLIBC_2_0)

	define_stub(raise)
	compat_stub(raise, raise, GLIBC_2_0)

        define_stub(read)
        compat_stub(read, read, GLIBC_2_0)
        compat_stub(read, __read, GLIBC_2_0)

        define_stub(recv)
        compat_stub(recv, recv, GLIBC_2_0)

        define_stub(recvfrom)
        compat_stub(recvfrom, recvfrom, GLIBC_2_0)

        define_stub(recvmsg)
        compat_stub(recvmsg, recvmsg, GLIBC_2_0)

        define_stub(send)
        compat_stub(send, send, GLIBC_2_0)
        compat_stub(send, __send, GLIBC_2_0)

        define_stub(sendmsg)
        compat_stub(sendmsg, sendmsg, GLIBC_2_0)

        define_stub(sendto)
        compat_stub(sendto, sendto, GLIBC_2_0)

        define_stub(sigwait)
        compat_stub(sigwait, sigwait, GLIBC_2_0)

        define_stub(tcdrain)
        compat_stub(tcdrain, tcdrain, GLIBC_2_0)

        define_stub(wait)
        compat_stub(wait, wait, GLIBC_2_0)
        compat_stub(wait, __wait, GLIBC_2_0)

        define_stub(waitpid)
        compat_stub(waitpid, waitpid, GLIBC_2_0)

        define_stub(write)
        compat_stub(write, write, GLIBC_2_0)
        compat_stub(write, __write, GLIBC_2_0)

        define_stub(pthread_kill)
        compat_stub(pthread_kill, pthread_kill, GLIBC_2_0)

#endif

/* The off64_t functions were added in glibc 2.2, but some architectures
   had 64-bit off_t before that.  On those architectures, the unsuffixed
   lseek, open, etc should be additional aliases for the suffixed ones.  */

#if SHLIB_COMPAT (libpthread, GLIBC_2_2, GLIBC_2_28)    \
  || (SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_28)  \
      && defined __OFF_T_MATCHES_OFF64_T)

#ifndef __OFF_T_MATCHES_OFF64_T
        define_stub(lseek)
        compat_stub(lseek, lseek, GLIBC_2_0)
        compat_stub(lseek, __lseek, GLIBC_2_0)

        define_stub(open)
        compat_stub(open, open, GLIBC_2_0)
        compat_stub(open, __open, GLIBC_2_0)
#endif

        define_stub(lseek64)
        compat_stub(lseek64, lseek64, GLIBC_2_2)
# ifdef __OFF_T_MATCHES_OFF64_T
        compat_stub(lseek64, lseek, GLIBC_2_0)
        compat_stub(lseek64, __lseek, GLIBC_2_0)
# endif

        define_stub(open64)
        compat_stub(open64, open64, GLIBC_2_2)
        compat_stub(open64, __open64, GLIBC_2_2)
# ifdef __OFF_T_MATCHES_OFF64_T
        compat_stub(open64, open, GLIBC_2_0)
        compat_stub(open64, __open, GLIBC_2_0)
# endif

#endif

/* pread and pwrite were added in the same release as pread64 and pwrite64,
   and do not have exposed __ variants.  */

#if SHLIB_COMPAT (libpthread, GLIBC_2_2, GLIBC_2_28)

# ifndef __OFF_T_MATCHES_OFF64_T
        define_stub(pread)
        compat_stub(pread, pread, GLIBC_2_2)

        define_stub(pwrite)
        compat_stub(pwrite, pwrite, GLIBC_2_2)
# endif

        define_stub(pread64)
        compat_stub(pread64, pread64, GLIBC_2_2)
        compat_stub(pread64, __pread64, GLIBC_2_2)
#ifdef __OFF_T_MATCHES_OFF64_T
        compat_stub(pread64, pread, GLIBC_2_2)
#endif

        define_stub(pwrite64)
        compat_stub(pwrite64, pwrite64, GLIBC_2_2)
        compat_stub(pwrite64, __pwrite64, GLIBC_2_2)
#ifdef __OFF_T_MATCHES_OFF64_T
        compat_stub(pwrite64, pwrite, GLIBC_2_2)
#endif

#endif
