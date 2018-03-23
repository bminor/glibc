/* ABI compatibility stubs for libpthread.  PowerPC/Linux version.
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

/* AltiVec support was added to setjmp/longjmp in 2.3.4,
   requiring a new symbol version.  */
#define LONGJMP_2_0_DEST       __novmx__libc_siglongjmp
#define LONGJMP_2_0_UNTIL_VER  GLIBC_2_3_4

#include <sysdeps/generic/pt-compat-stubs.S>

#if SHLIB_COMPAT (libpthread, GLIBC_2_3_4, GLIBC_2_28)

	define_stub_with_dest(vmx_longjmp, __libc_longjmp)
	compat_stub(vmx_longjmp, longjmp, GLIBC_2_3_4)
	compat_stub(vmx_longjmp, siglongjmp, GLIBC_2_3_4)

#endif
