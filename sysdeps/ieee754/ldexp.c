/* Copyright (C) 1992 Free Software Foundation, Inc.
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

/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <ansidecl.h>
#include <math.h>
#include "ieee754.h"

double
DEFUN(ldexp, (x, exp),
      double x AND int exp)
{
  union ieee754_double u;
  u.d = x;
#define	x u.d

  if (__finite (x))
    {
      unsigned int k;

      if (exp < -2100)
	return 3e-39 * 3e-39;	/* Cause underflow.  */
      else if (n > 2100)
	return 1.7e308 + 1.7e308; /* Cause overflow.  */

      if (u.ieee.exponent == 0)
	return ldexp (x * ldexp (1.0, 54), exp - 54);

      k = u.ieee.exponent + exp;
      if (k > 0)
	{
	  if (k < 2047)
	    u.ieee.exponent = k;
	  else
	    return 1.7e308 + 1.7e308; /* Cause overflow.  */
	}
      else if (k > -54)
	{
	  /* Gradual underflow.  */
	  u.ieee.exponent = 1;
	  x *= ldexp (1.0, k - 1);
	}
      else
	return 3e-39 * 3e-39;	/* Cause underflow.  */
    }

  return x;
}
