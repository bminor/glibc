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

/* Use the same register names and assignments as memset.  */

	.arch armv8.5-a
	.arch_extension memtag

/* NB, only supported on variants with 64-bit pointers.  */

/* FIXME: This is a minimal implementation.  We could do better than
   this for larger values of COUNT.  */

#define dstin x0
#define count x1
#define dst   x2

ENTRY_ALIGN(__libc_mtag_tag_region, 6)

	mov	dst, dstin
L(loop):
	stg	dst, [dst], #16
	subs	count, count, 16
	bne	L(loop)
#if 0
	/* This is not currently needed, since for now we are only called
	   to tag memory that is taggable.  */
	ldg	dstin, [dstin] // Recover the tag created (might be untagged).
#endif
	ret
END (__libc_mtag_tag_region)
#endif /* USE_MTAG */
