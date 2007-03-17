/* Copyright (C) 1998, 2000, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <math.h>

/* Use the -inf rounding mode conversion instructions to implement
   ceil, via something akin to -floor(-x).  This is much faster than
   playing with the fpcr to achieve +inf rounding mode.  */

float
__ceilf (float x)
{
  float two23 = copysignf (0x1.0p23, x);
  float r, tmp;
  
  __asm (
#ifdef _IEEE_FP_INEXACT
	 "adds/suim %2, %3, %1\n\tsubs/suim %1, %3, %0"
#else
	 "adds/sum %2, %3, %1\n\tsubs/sum %1, %3, %0"
#endif
	 : "=&f"(r), "=&f"(tmp)
	 : "f"(-x), "f"(-two23));

  /* Fix up the negation we did above, as well as handling -0 properly. */
  return copysignf (r, x);
}

weak_alias (__ceilf, ceilf)
