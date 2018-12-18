/* Set a block of memory to some byte value.  31/64 bit S/390 version.
   Copyright (C) 2001-2018 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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
     %r2 = address of memory area
     %r3 = byte to fill memory with
     %r4 = number of bytes to fill.  */

       .text

#if defined __s390x__
# define LTGR	ltgr
# define CGHI	cghi
# define LGR	lgr
# define AGHI	aghi
# define BRCTG	brctg
#else
# define LTGR	ltr
# define CGHI	chi
# define LGR	lr
# define AGHI	ahi
# define BRCTG	brct
#endif /* ! defined __s390x__  */

#ifdef USE_MULTIARCH
ENTRY(__memset_default)
#else
ENTRY(memset)
#endif
#if defined __s390x__
	.machine "z900"
#else
	.machine "g5"
#endif /* ! defined __s390x__  */
	LTGR    %r4,%r4
	je      .L_Z900_G5_4
	stc     %r3,0(%r2)
	CGHI    %r4,1
	LGR     %r1,%r2
	je      .L_Z900_G5_4
	AGHI    %r4,-2
#if defined __s390x__
	larl    %r5,.L_Z900_G5_18
	srlg    %r3,%r4,8
# define Z900_G5_EX_D 0
#else
	basr    %r5,0
.L_Z900_G5_19:
# define Z900_G5_EX_D .L_Z900_G5_18-.L_Z900_G5_19
	lr      %r3,%r4
	srl     %r3,8
#endif /* ! defined __s390x__  */
	LTGR    %r3,%r3
	jne     .L_Z900_G5_14
.L_Z900_G5_3:
	ex      %r4,Z900_G5_EX_D(%r5)
.L_Z900_G5_4:
	br      %r14
.L_Z900_G5_14:
	mvc     1(256,%r1),0(%r1)
	la      %r1,256(%r1)
	BRCTG   %r3,.L_Z900_G5_14
	j       .L_Z900_G5_3
.L_Z900_G5_18:
	mvc     1(1,%r1),0(%r1)
#ifdef USE_MULTIARCH
END(__memset_default)
#else
END(memset)
libc_hidden_builtin_def (memset)
#endif

#undef LTGR
#undef CGHI
#undef LGR
#undef AGHI
#undef BRCTG
