/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <syscall.h>

/* Not that using a `PASTE' macro loses.  */
#ifdef	__STDC__
#define	SYSCALL__(name)	PSEUDO (__##name, name)
#else
#define	SYSCALL__(name)	PSEUDO (__/**/name, name)
#endif
#define	SYSCALL(name)	PSEUDO (name, name)

/* Machine-dependent sysdep.h files are expected to define the macro
   PSEUDO (function_name, syscall_name) to emit assembly code to define the
   C-callable function FUNCTION_NAME to do system call SYSCALL_NAME.
   r0 and r1 are the system call outputs.  movl should be defined as
   an instruction such that "movl r1, r0" works.  ret should be defined
   as the return instruction.  */


#ifndef	 __GNU_STAB__
#define	 ___errno	_errno
#endif
