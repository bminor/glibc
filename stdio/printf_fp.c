/* Floating-point printing for `printf'.
   This is an implementation of a restricted form of the `Dragon4'
   algorithm described in "How to Print Floating-Point Numbers Accurately",
   by Guy L. Steele, Jr. and Jon L. White, presented at the ACM SIGPLAN '90
   Conference on Programming Language Design and Implementation.

Copyright (C) 1992 Free Software Foundation, Inc.
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
#include <ctype.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <localeinfo.h>

#include <printf.h>

#define	outchar(x)							      \
  do									      \
    {									      \
      register CONST int outc = (x);					      \
      if (putc(outc, s) == EOF)						      \
	return -1;							      \
      else								      \
	++done;								      \
    } while (0)

int
DEFUN(__printf_fp, (s, info, args),
      FILE *s AND CONST struct printf_info *info AND va_list *args)
{
  int done = 0;

  /* Decimal point character.  */
  CONST char *CONST decimal = _numeric_info->decimal_point;

  LONG_DOUBLE fpnum;		/* Input.  */
  int is_neg;

  LONG_DOUBLE f;		/* Fraction.  */
  int e;			/* Base-2 exponent of the input.  */
  CONST int p = DBL_MANT_DIG;	/* Internal precision.  */
  LONG_DOUBLE scale, scale10;	/* Scale factor.  */
  LONG_DOUBLE loerr, hierr;	/* Potential error in the fraction.  */
  int k;			/* Digits to the left of the decimal point.  */
  int cutoff;			/* Where to stop generating digits.  */
  LONG_DOUBLE r;		/* Remainder.  */
  int roundup;
  int low, high;
  char digit;

  char type = tolower (info->spec);
  CONST char pad = info->pad;
  int prec = info->prec;
  int width = info->width;


  /* Fetch the argument value.  */
  if (info->is_long_double)
    fpnum = va_arg (*args, LONG_DOUBLE);
  else
    fpnum = (LONG_DOUBLE) va_arg (*args, double);

#ifdef	HANDLE_SPECIAL
  /* Allow for machine-dependent (or floating point format-dependent) code.  */
  HANDLE_SPECIAL (done, s, info, fpnum);
#endif

#ifndef	IS_NEGATIVE
#define	IS_NEGATIVE(num)	((num) < 0)
#endif

  is_neg = IS_NEGATIVE (fpnum);
  if (is_neg)
    fpnum = - fpnum;

  if (prec == -1)
    prec = 6;
  
  if (type == 'g')
    {
      if (prec == 0)
	prec = 1;

      if (fpnum == 0 || fpnum >= 1e-4 && fpnum < pow(10.0, (double) prec))
	type = 'f';

      /* For 'g'/'G' format, the precision specifies "significant digits",
	 not digits to come after the decimal point.  */
      --prec;
    }
  
