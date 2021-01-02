/* Optimized strcpy implementation for PowerPC64/POWER9.
   Copyright (C) 2020-2021 Free Software Foundation, Inc.
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

#include <sysdep.h>

#ifdef USE_AS_STPCPY
# ifndef STPCPY
#   define FUNC_NAME __stpcpy
# else
#   define FUNC_NAME STPCPY
# endif
#else
# ifndef STRCPY
#  define FUNC_NAME strcpy
# else
#  define FUNC_NAME STRCPY
# endif
#endif  /* !USE_AS_STPCPY  */

/* Implements the function

   char * [r3] strcpy (char *dest [r3], const char *src [r4])

   or

   char * [r3] stpcpy (char *dest [r3], const char *src [r4])

   if USE_AS_STPCPY is defined.

   The implementation can load bytes past a null terminator, but only
   up to the next 16B boundary, so it never crosses a page.  */

.machine power9
ENTRY_TOCLESS (FUNC_NAME, 4)
	CALL_MCOUNT 2

	/* NULL string optimisation  */
	lbz	r0,0(r4)
	stb	r0,0(r3)
	cmpwi	r0,0
	beqlr

	addi	r4,r4,1
	addi	r11,r3,1

	vspltisb v18,0		/* Zeroes in v18  */

	neg	r5,r4
	rldicl	r9,r5,0,60	/* How many bytes to get source 16B aligned?  */

	/* Get source 16B aligned  */
	lvx	v0,0,r4
	lvsr	v1,0,r4
	vperm	v0,v18,v0,v1

	vcmpequb v6,v0,v18	/* 0xff if byte is NULL, 0x00 otherwise  */
	vctzlsbb r7,v6		/* Number of trailing zeroes  */
	addi	r8,r7,1		/* Add null terminator  */

	/* r8 = bytes including null
	   r9 = bytes to get source 16B aligned
	   if r8 > r9
	      no null, copy r9 bytes
	   else
	      there is a null, copy r8 bytes and return.  */
	cmpd	r8,r9
	bgt	L(no_null)

	sldi	r10,r8,56	/* stxvl wants size in top 8 bits  */
	stxvl	32+v0,r11,r10	/* Partial store  */

#ifdef USE_AS_STPCPY
	/* stpcpy returns the dest address plus the size not counting the
	   final '\0'.  */
	add	r3,r11,r7
#endif
	blr

L(no_null):
	sldi	r10,r9,56	/* stxvl wants size in top 8 bits  */
	stxvl	32+v0,r11,r10	/* Partial store  */

	add	r4,r4,r9
	add	r11,r11,r9

L(loop):
	lxv	32+v0,0(r4)
	vcmpequb. v6,v0,v18	/* Any zero bytes?  */
	bne	cr6,L(tail1)

	lxv	32+v1,16(r4)
	vcmpequb. v6,v1,v18	/* Any zero bytes?  */
	bne	cr6,L(tail2)

	lxv	32+v2,32(r4)
	vcmpequb. v6,v2,v18	/* Any zero bytes?  */
	bne	cr6,L(tail3)

	lxv	32+v3,48(r4)
	vcmpequb. v6,v3,v18	/* Any zero bytes?  */
	bne	cr6,L(tail4)

	stxv	32+v0,0(r11)
	stxv	32+v1,16(r11)
	stxv	32+v2,32(r11)
	stxv	32+v3,48(r11)

	addi	r4,r4,64
	addi	r11,r11,64

	b	L(loop)

L(tail1):
	vctzlsbb r8,v6
	addi	r9,r8,1
	sldi	r9,r9,56	/* stxvl wants size in top 8 bits  */
	stxvl	32+v0,r11,r9
#ifdef USE_AS_STPCPY
	/* stpcpy returns the dest address plus the size not counting the
	   final '\0'.  */
	add	r3,r11,r8
#endif
	blr

L(tail2):
	stxv	32+v0,0(r11)
	vctzlsbb r8,v6		/* Number of trailing zeroes  */
	addi	r9,r8,1		/* Add null terminator  */
	sldi	r10,r9,56	/* stxvl wants size in top 8 bits  */
	addi	r11,r11,16
	stxvl	32+v1,r11,r10	/* Partial store  */
#ifdef USE_AS_STPCPY
	/* stpcpy returns the dest address plus the size not counting the
	   final '\0'.  */
	add	r3,r11,r8
#endif
	blr

L(tail3):
	stxv	32+v0,0(r11)
	stxv	32+v1,16(r11)
	vctzlsbb r8,v6		/* Number of trailing zeroes  */
	addi	r9,r8,1		/* Add null terminator  */
	sldi	r10,r9,56	/* stxvl wants size in top 8 bits  */
	addi	r11,r11,32
	stxvl	32+v2,r11,r10	/* Partial store  */
#ifdef USE_AS_STPCPY
	/* stpcpy returns the dest address plus the size not counting the
	   final '\0'.  */
	add	r3,r11,r8
#endif
	blr

L(tail4):
	stxv	32+v0,0(r11)
	stxv	32+v1,16(r11)
	stxv	32+v2,32(r11)
	vctzlsbb r8,v6		/* Number of trailing zeroes  */
	addi	r9,r8,1		/* Add null terminator  */
	sldi	r10,r9,56	/* stxvl wants size in top 8 bits  */
	addi	r11,r11,48
	stxvl	32+v3,r11,r10	/* Partial store  */
#ifdef USE_AS_STPCPY
	/* stpcpy returns the dest address plus the size not counting the
	   final '\0'.  */
	add	r3,r11,r8
#endif
	blr
END (FUNC_NAME)
#ifndef USE_AS_STPCPY
libc_hidden_builtin_def (strcpy)
#endif
