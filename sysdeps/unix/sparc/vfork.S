/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#define ASM

#include <ansidecl.h>
#include <sysdep.h>

#ifndef	SYS_vfork
#define	SYS_vfork	66
#endif

/* Clone the calling process, but without copying the whole address space.
   The the calling process is suspended until the the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
.global ___vfork
___vfork:
	/* Put the syscall number in %g1 and trap into the kernel.  */
	mov SYS_vfork, %g1
	ta 0

	bcs error		/* Branch forward if it failed.  */

	/* It succeeded.  See which fork we're in.  */
	tst %o1			/* If %o1 is zero, we're the parent.  */
	bne,a end
	/* We're the child, so return zero.  */
	clr %o0

end:
	retl
	nop

error:
	/* We got an error.  Set errno with the code in %o0 and return -1.  */
	sethi %hi(_errno), %g1
	st %o0, [%g1 + %lo(_errno)]
	retl
	sub %g0, 1, %o0
