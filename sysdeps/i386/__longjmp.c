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


#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#endif


/* Jump to the position specified by ENV, causing the
   setjmp call there to return VAL, or 1 if VAL is 0.  */
__NORETURN
void
DEFUN(__longjmp, (env, val), CONST jmp_buf env AND int val)
{
  register long int bp asm("%ebp"), ax asm("%eax");

  if (env[0].__bp != bp)
    {
      asm("movl %0, %%ebx" : : "g" (env[0].__bx) : "ebx");
      asm("movl %0, %%esi" : : "g" (env[0].__si) : "esi");
      asm("movl %0, %%edi" : : "g" (env[0].__di) : "edi");
      asm("movl %0, %%ebp" : : "g" (env[0].__bp) : "ebp");
      asm("movl %0, %%esp" : : "g" (env[0].__sp) : "esp");
    }

  ax = val == 0 ? 1 : val;
  asm volatile ("jmp %*%0" : : "g" (env[0].__dx));

  /* NOTREACHED */
  abort ();
}
