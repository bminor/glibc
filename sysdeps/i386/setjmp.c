/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <errno.h>
#include <setjmp.h>


/* Save the current program position in ENV and return 0.  */
int
DEFUN(__setjmp, (env), jmp_buf env)
{
  asm ("movl %%ebx, %0" : "=g" (env[0].__bx));
  asm ("movl %%esi, %0" : "=g" (env[0].__si));
  asm ("movl %%edi, %0" : "=g" (env[0].__di));
  asm ("movl %%ebp, %0" : "=g" (env[0].__bp));
  asm volatile ("popl %%edx\n"
		"movl %%edx, %0\n"
		"movl %%esp, %1\n"
		"xorl %%eax, %%eax\n"
		"jmp *%%edx\n" :
		"=g" (env[0].__dx), "=g" (env[0].__sp));

  /* NOTREACHED */
  return 0;
}
