/* Copyright (C) 1998, 1999, 2000, 2006, 2007 Free Software Foundation, Inc.
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
#include <math_ldbl_opt.h>


/* Use the -inf rounding mode conversion instructions to implement floor.  */

double
__floor (double x)
{
  double two52 = copysign (0x1.0p52, x);
  double r, tmp;
  
  __asm (
#ifdef _IEEE_FP_INEXACT
	 "addt/suim %2, %3, %1\n\tsubt/suim %1, %3, %0"
#else
	 "addt/sum %2, %3, %1\n\tsubt/sum %1, %3, %0"
#endif
	 : "=&f"(r), "=&f"(tmp)
	 : "f"(x), "f"(two52));

  /* floor(-0) == -0, and in general we'll always have the same
     sign as our input.  */
  return copysign (r, x);
}

weak_alias (__floor, floor)
#ifdef NO_LONG_DOUBLE
strong_alias (__floor, __floorl)
weak_alias (__floor, floorl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __floor, floorl, GLIBC_2_0);
#endif
