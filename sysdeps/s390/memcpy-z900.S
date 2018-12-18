/* memcpy - copy a block from source to destination.  31/64 bit S/390 version.
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
#include <ifunc-memcpy.h>

/* INPUT PARAMETERS
     %r2 = address of destination memory area
     %r3 = address of source memory area
     %r4 = number of bytes to copy.  */

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

#if HAVE_MEMCPY_Z900_G5
ENTRY(MEMPCPY_Z900_G5)
# if defined __s390x__
	.machine "z900"
# else
	.machine "g5"
# endif /* ! defined __s390x__  */
	LGR     %r1,%r2             # Use as dest
	la      %r2,0(%r4,%r2)      # Return dest + n
	j	.L_Z900_G5_start
END(MEMPCPY_Z900_G5)

ENTRY(MEMCPY_Z900_G5)
# if defined __s390x__
	.machine "z900"
# else
	.machine "g5"
# endif /* ! defined __s390x__  */
	LGR     %r1,%r2             # r1: Use as dest ; r2: Return dest
.L_Z900_G5_start:
	LTGR    %r4,%r4
	je      .L_Z900_G5_4
	AGHI    %r4,-1
# if defined __s390x__
	srlg	%r5,%r4,8
# else
	lr	%r5,%r4
	srl	%r5,8
# endif /* ! defined __s390x__  */
	LTGR    %r5,%r5
	jne     .L_Z900_G5_13
.L_Z900_G5_3:
# if defined __s390x__
	larl    %r5,.L_Z900_G5_15
#  define Z900_G5_EX_D 0
# else
	basr    %r5,0
.L_Z900_G5_14:
#  define Z900_G5_EX_D .L_Z900_G5_15-.L_Z900_G5_14
# endif /* ! defined __s390x__  */
	ex      %r4,Z900_G5_EX_D(%r5)
.L_Z900_G5_4:
	br      %r14
.L_Z900_G5_13:
	CGHI	%r5,4096            # Switch to mvcle for copies >1MB
	jh      __memcpy_mvcle
.L_Z900_G5_12:
	mvc     0(256,%r1),0(%r3)
	la      %r1,256(%r1)
	la      %r3,256(%r3)
	BRCTG   %r5,.L_Z900_G5_12
	j       .L_Z900_G5_3
.L_Z900_G5_15:
	mvc     0(1,%r1),0(%r3)
END(MEMCPY_Z900_G5)
#endif /* HAVE_MEMCPY_Z900_G5  */

ENTRY(__memcpy_mvcle)
	# Using as standalone function will result in unexpected
	# results since the length field is incremented by 1 in order to
	# compensate the changes already done in the functions above.
	LGR     %r0,%r2             # backup return dest [ + n ]
	AGHI    %r4,1               # length + 1
	LGR     %r5,%r4             # source length
	LGR     %r4,%r3             # source address
	LGR     %r2,%r1             # destination address
	LGR     %r3,%r5             # destination length = source length
.L_MVCLE_1:
	mvcle   %r2,%r4,0           # thats it, MVCLE is your friend
	jo      .L_MVCLE_1
	LGR     %r2,%r0             # return destination address
	br      %r14
END(__memcpy_mvcle)

#undef LTGR
#undef CGHI
#undef LGR
#undef AGHI
#undef BRCTG

#if HAVE_MEMCPY_Z10
ENTRY(MEMPCPY_Z10)
	.machine "z10"
	.machinemode "zarch_nohighgprs"
	lgr     %r1,%r2         # Use as dest
	la      %r2,0(%r4,%r2)  # Return dest + n
	j	.L_Z10_start
END(MEMPCPY_Z10)

ENTRY(MEMCPY_Z10)
	.machine "z10"
	.machinemode "zarch_nohighgprs"
	lgr     %r1,%r2         # r1: Use as dest ; r2: Return dest
.L_Z10_start:
# if !defined __s390x__
	llgfr	%r4,%r4
# endif /* !defined __s390x__  */
	cgije   %r4,0,.L_Z10_4
	aghi    %r4,-1
	srlg    %r5,%r4,8
	cgijlh  %r5,0,.L_Z10_13
.L_Z10_3:
	exrl    %r4,.L_Z10_15
.L_Z10_4:
	br      %r14
.L_Z10_13:
	cgfi    %r5,65535	# Switch to mvcle for copies >16MB
	jh      __memcpy_mvcle
.L_Z10_12:
	pfd     1,768(%r3)
	pfd     2,768(%r1)
	mvc     0(256,%r1),0(%r3)
	la      %r1,256(%r1)
	la      %r3,256(%r3)
	brctg   %r5,.L_Z10_12
	j       .L_Z10_3
.L_Z10_15:
	mvc     0(1,%r1),0(%r3)
END(MEMCPY_Z10)
#endif /* HAVE_MEMCPY_Z10  */

#if HAVE_MEMCPY_Z196
ENTRY(MEMPCPY_Z196)
	.machine "z196"
	.machinemode "zarch_nohighgprs"
	lgr     %r1,%r2         # Use as dest
	la      %r2,0(%r4,%r2)  # Return dest + n
	j	.L_Z196_start
END(MEMPCPY_Z196)

ENTRY(MEMCPY_Z196)
	.machine "z196"
	.machinemode "zarch_nohighgprs"
	lgr     %r1,%r2         # r1: Use as dest ; r2: Return dest
.L_Z196_start:
# if !defined __s390x__
	llgfr	%r4,%r4
# endif /* !defined __s390x__  */
	ltgr    %r4,%r4
	je      .L_Z196_4
	aghi    %r4,-1
	srlg    %r5,%r4,8
	ltgr    %r5,%r5
	jne     .L_Z196_5
.L_Z196_3:
	exrl    %r4,.L_Z196_14
.L_Z196_4:
	br      %r14
.L_Z196_5:
	cgfi    %r5,262144      # Switch to mvcle for copies >64MB
	jh      __memcpy_mvcle
.L_Z196_2:
	pfd     1,768(%r3)
	pfd     2,768(%r1)
	mvc     0(256,%r1),0(%r3)
	aghi    %r5,-1
	la      %r1,256(%r1)
	la      %r3,256(%r3)
	jne     .L_Z196_2
	j       .L_Z196_3
.L_Z196_14:
	mvc     0(1,%r1),0(%r3)
END(MEMCPY_Z196)
#endif /* HAVE_MEMCPY_Z196  */

#if ! HAVE_MEMCPY_IFUNC
/* If we don't use ifunc, define an alias for mem[p]cpy here.
   Otherwise see sysdeps/s390/mem[p]cpy.c.  */
strong_alias (MEMCPY_DEFAULT, memcpy)
strong_alias (MEMPCPY_DEFAULT, __mempcpy)
weak_alias (__mempcpy, mempcpy)
#endif

#if defined SHARED && IS_IN (libc)
/* Defines the internal symbols.
   Compare to libc_hidden_[builtin_]def (mem[p]cpy) in string/mem[p]cpy.c.  */
strong_alias (MEMCPY_DEFAULT, __GI_memcpy)
strong_alias (MEMPCPY_DEFAULT, __GI_mempcpy)
strong_alias (MEMPCPY_DEFAULT, __GI___mempcpy)
#endif
