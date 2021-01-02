/* Copyright (C) 2020-2021 Free Software Foundation, Inc.

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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

#ifdef USE_MTAG

	.arch armv8.5-a
	.arch_extension memtag

/* NB, only supported on variants with 64-bit pointers.  */

#define ptr x0
#define xset x1

ENTRY(__libc_mtag_new_tag)
	// Guarantee that the new tag is not the same as now.
	gmi	xset, ptr, xzr
	irg	ptr, ptr, xset
	ret
END (__libc_mtag_new_tag)
#endif /* USE_MTAG */
