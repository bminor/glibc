/* Compatibility stubs for functions formerly exposed by libpthread.
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

        compat_text_section

#ifndef SIBCALL_ENTRY
# define SIBCALL_ENTRY(name) ENTRY(name)
#endif

#define define_stub(name)						      \
  define_stub_1(__pstub_##name, __libc_##name)
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
