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

#include <ansidecl.h>
#include <errno.h>
#include <setjmp.h>


/* Save the current program position in ENV and return 0.  */
int
DEFUN(__setjmp, (env), jmp_buf env)
{
  errno = ENOSYS;

  /* No way to signal failure.	*/
  return 0;
}


#ifdef	 __GNU_STAB__

#include <gnu-stabs.h>

stub_warning(__setjmp);

#endif	/* GNU stabs.  */
