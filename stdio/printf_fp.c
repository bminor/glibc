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

  LONG_DOUBLE fpnum;
  int is_neg;

  LONG_DOUBLE f;		/* Fraction.  */
  int e;			/* Base-2 exponent of the input.  */
  const int p = DBL_MANT_DIG;	/* Internal precision.  */
  LONG_DOUBLE scale, scale10;	/* Scale factor.  */
  LONG_DOUBLE loerr, hierr;	/* Potential error in the fraction.  */
  int k;			/* Digits to the left of the decimal point.  */
  int cutoff;			/* Where to stop generating digits.  */
  LONG_DOUBLE r;		/* Remainder.  */
  int roundup;
  int low, high;
  char digit;

  char type = tolower(info->spec);
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
  
  if (fpnum == 0.0)
    /* Special case for zero.
       The general algorithm does not work for zero.  */
    puts ("0.0");
  else
    {
      /* Split the number into a fraction and base-2 exponent.  */
      f = frexp (fpnum, &e);

      /* Scale the fractional part by the highest possible number of
	 significant bits of fraction.  We want to represent the
	 fractional part as a (very) large integer.  */
      f = ldexp (f, p);

      cutoff = 0;

      roundup = 0;

      if (e > p)
	{
	  /* The exponent is bigger than the number of fractional digits.  */
	  r = ldexp (f, e - p);
	  scale = 1;
	  /* The number is (E - P) powers of two larger than
	     the fraction can represent; this is the potential error.  */
	  loerr = ldexp (1.0, e - p);
	}
      else
	{
	  /* The number of fractional digits is greater than the exponent.
	     Scale by the difference factors of two.  */
	  r = f;
	  scale = ldexp (1.0, p - e);
	  loerr = 1.0;
	}
      hierr = loerr;

      /* Fixup.  */

      if (f == ldexp (1.0, p - 1))
	{
	  /* Account for unequal gaps.  */
	  hierr = ldexp (hierr, 1);
	  r = ldexp (r, 1);
	  scale = ldexp (scale, 1);
	}

      tenth_scale = ceil (scale / 10.0);
      k = 0;
      while (r < tenth_scale)
	{
	  --k;
	  r *= 10;
	  loerr *= 10;
	  hierr *= 10;
	}
      do
	{
	  while ((2 * r) + hierr >= 2 * scale)
	    {
	      scale *= 10;
	      ++k;
	    }

	  /* Perform any necessary adjustment of loerr and hierr to
	     take into account the formatting requirements.  */
	  cutoff = k;		/* CutOffMode == "normal" */
	} while ((2 * r) + hierr >= 2 * scale);

      /* End Fixup.  */

      /* First digit.  */
      {
	--k;
	digit = '0' + (unsigned int) floor ((r * 10) / scale);
	r = fmod (r * 10, scale);
	loerr *= 10;
	hierr *= 10;
	
	low = 2 * r < loerr;
	if (roundup)
	  high = 2 * r >= (2 * scale) - hierr;
	else
	  high = 2 * r > (2 * scale) - hierr;
      }

      if (k < 0)
	{
	  int j;
	  putchar ('0');
	  putchar ('.');
	  for (j = 0; j >= k; --j)
	    putchar ('0');
	}
      
      if (low || high || k == cutoff)
	{
	  if ((high && !low) || (2 * r > scale))
	    ++digit;
	  putchar (digit);
	}
      else
	while (1)
	  {
	    putchar (digit);
	    if (k == 0)
	      putchar ('.');
	    
	    --k;
	    digit = '0' + (unsigned int) floor ((r * 10) / scale);
	    r = fmod (r * 10, scale);
	    loerr *= 10;
	    hierr *= 10;
	    
	    low = 2 *r < loerr;
	    if (roundup)
	      high = 2 * r >= (2 * scale) - hierr;
	    else
	      high = 2 *r > (2 * scale) - hierr;
	    
	    if (low || high || k == cutoff)
	      {
		if ((high && !low) || (2 * r > scale))
		  ++digit;
		putchar (digit);
		break;
	      }
	  }

      putchar ('\n');
    }
}
