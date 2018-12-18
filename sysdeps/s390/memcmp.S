/* memcmp - compare two memory blocks.  31/64 bit S/390 version.
   Copyright (C) 2012-2018 Free Software Foundation, Inc.
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


#include <sysdep.h>
#include "asm-syntax.h"

/* INPUT PARAMETERS
     %r2 = address of first memory area
     %r3 = address of second memory area
     %r4 = number of bytes to compare.  */

       .text
#if defined __s390x__
# define LTGR	ltgr
# define AGHI	aghi
# define BRCTG	brctg
#else
# define LTGR	ltr
# define AGHI	ahi
# define BRCTG	brct
#endif /* ! defined __s390x__  */

#ifdef USE_MULTIARCH
ENTRY(__memcmp_default)
#else
ENTRY(memcmp)
#endif
#if defined __s390x__
	.machine "z900"
#else
	.machine "g5"
	basr    %r5,0
.L_Z900_G5_16:
# define Z900_G5_EX_D .L_Z900_G5_15-.L_Z900_G5_16
#endif /* ! defined __s390x__  */
	LTGR    %r4,%r4
	je      .L_Z900_G5_4
	AGHI    %r4,-1
#if defined __s390x__
	srlg    %r1,%r4,8
	larl    %r5,.L_Z900_G5_15
# define Z900_G5_EX_D 0
#else
	lr	%r1,%r4
	srl	%r1,8
#endif /* ! defined __s390x__  */
	LTGR    %r1,%r1
	jne     .L_Z900_G5_12
.L_Z900_G5_3:
	ex      %r4,Z900_G5_EX_D(%r5)
.L_Z900_G5_4:
	ipm     %r2
#if defined __s390x__
	sllg    %r2,%r2,34
	srag    %r2,%r2,62
#else
	sll     %r2,2
	sra     %r2,30
#endif /* ! defined __s390x__  */
	br      %r14
.L_Z900_G5_12:
	clc     0(256,%r3),0(%r2)
	jne     .L_Z900_G5_4
	la      %r3,256(%r3)
	la      %r2,256(%r2)
	BRCTG   %r1,.L_Z900_G5_12
	j       .L_Z900_G5_3
.L_Z900_G5_15:
	clc     0(1,%r3),0(%r2)
#ifdef USE_MULTIARCH
END(__memcmp_default)
#else
END(memcmp)
libc_hidden_builtin_def (memcmp)
weak_alias (memcmp, bcmp)
#endif

#undef LTGR
#undef AGHI
#undef BRCTG
