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

#include "DEFS.h"
#include <machine/trap.h>

FUNC (___longjmp)
	/* Do a "flush register windows trap".  The trap handler in the
	   kernel writes all the register windows to their stack slots, and
	   marks them all as invalid (needing to be sucked up from the
	   stack when used).  This ensures that all information needed to
	   unwind to these callers is in memory, not in the register
	   windows.  */
	ta ST_FLUSH_WINDOWS
	ld [%o0], %o7		/* Return PC.  */
	ld [%o0 + 4], %fp
	sub %fp, 64, %sp

	/* if (%o1 == 0) %o1 = 1; */
	tst %o1
	bz,a 0f
	mov 1, %o1

0:	retl
	/* On the way out, put the return value in %o0.  */
	restore %o1, 0, %o0
