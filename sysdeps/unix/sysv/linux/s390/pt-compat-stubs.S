/* ABI compatibility stubs for libpthread.  S/390/Linux version.
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

#include <sysdeps/generic/pt-compat-stubs.S>

#if SHLIB_COMPAT (libpthread, GLIBC_2_19, GLIBC_2_20)
/* In glibc release 2.19 new versions of longjmp-functions were introduced,
   but were reverted before 2.20. Thus both versions are the same function.  */

	compat_stub(longjmp, longjmp, GLIBC_2_19)
	compat_stub(longjmp, siglongjmp, GLIBC_2_19)

#endif
