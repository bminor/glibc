/* Floating-point printing for `printf'.
   This is an implementation of a restricted form of the `Dragon4'
   algorithm described in "How to Print Floating-Point Numbers Accurately",
   by Guy L. Steele, Jr. and Jon L. White, presented at the ACM SIGPLAN '90
   Conference on Programming Language Design and Implementation.

Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
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
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <localeinfo.h>

#include <printf.h>

#define NDEBUG
#include <assert.h>

#define	outchar(x)							      \
  do									      \
    {									      \
      register CONST int outc = (x);					      \
      if (putc (outc, s) == EOF)					      \
	return -1;							      \
      else								      \
	++done;								      \
    } while (0)

#if FLT_RADIX != 2
 #error "FLT_RADIX != 2.  Write your own __printf_fp."
#endif

#undef alloca			/* gmp-impl.h defines it again.  */
#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

#ifndef NDEBUG
static void mpn_dump (const char *str, mp_limb *p, mp_size_t size);
#define MPN_DUMP(x,y,z) mpn_dump(x,y,z)
#else
#define MPN_DUMP(x,y,z)
#endif

extern mp_size_t __mpn_extract_double (mp_ptr res_ptr, mp_size_t size,
				       int *expt, int *is_neg,
				       double value);

/* We believe that these variables need as many bits as the largest binary
   exponent of a double.  But we are not confident, so we add a few words.  */
#define MPNSIZE ((DBL_MAX_EXP + BITS_PER_MP_LIMB - 1) / BITS_PER_MP_LIMB) + 3

#define MPN_VAR(name) mp_limb name[MPNSIZE]; mp_size_t name##size
#define MPN_ASSIGN(dst,src) \
  memcpy (dst, src, (dst##size = src##size) * sizeof (mp_limb))
#define MPN_POW2(dst, power) \
  do {									      \
    MPN_ZERO (dst, (power) / BITS_PER_MP_LIMB);				      \
    dst[(power) / BITS_PER_MP_LIMB] =					      \
      (mp_limb) 1 << (power) % BITS_PER_MP_LIMB;			      \
    dst##size = (power) / BITS_PER_MP_LIMB + 1;				      \
  } while (0)

/* Compare *normalized* mpn vars.  */
#define MPN_GT(u,v) \
  (u##size > v##size || (u##size == v##size && __mpn_cmp (u, v, u##size) > 0))
#define MPN_LT(u,v) \
  (u##size < v##size || (u##size == v##size && __mpn_cmp (u, v, u##size) < 0))
#define MPN_GE(u,v) \
  (u##size > v##size || (u##size == v##size && __mpn_cmp (u, v, u##size) >= 0))
#define MPN_LE(u,v) \
  (u##size < v##size || (u##size == v##size && __mpn_cmp (u, v, u##size) <= 0))
#define MPN_EQ(u,v) \
  (u##size == v##size && __mpn_cmp (u, v, u##size) == 0)
#define MPN_NE(u,v) \
  (!MPN_EQ(u,v))

int
DEFUN(__printf_fp, (s, info, args),
      FILE *s AND CONST struct printf_info *info AND va_list *args)
{
  mp_limb cy;

  int done = 0;

  /* Decimal point character.  */
  CONST char *CONST decimal = _numeric_info->decimal_point;

  LONG_DOUBLE fpnum;		/* Input.  */
  int is_neg;

  MPN_VAR (f);			/* Fraction.  */

  int e;			/* Base-2 exponent of the input.  */
  CONST int p = DBL_MANT_DIG;	/* Internal precision.  */
  MPN_VAR (scale); MPN_VAR (scale2); MPN_VAR (scale10); /* Scale factor.  */
  MPN_VAR (loerr); MPN_VAR (hierr); /* Potential error in the fraction.  */
  int k;			/* Digits to the left of the decimal point.  */
  int cutoff;			/* Where to stop generating digits.  */
  MPN_VAR (r); MPN_VAR (r2); MPN_VAR (r10); /* Remainder.  */
  int roundup;
  int low, high;
  char digit;

  MPN_VAR (tmp);		/* Scratch space.  */

  int j;

  char type = tolower (info->spec);
  int prec = info->prec;
  int width = info->width;

  /* This algorithm has the nice property of not needing a buffer.
     However, to get the padding right for %g format, we need to know
     the length of the number before printing it.  */

#ifndef	LDBL_DIG
#define	LDBL_DIG	DBL_DIG
#endif
#ifndef	LDBL_MAX_10_EXP
#define	LDBL_MAX_10_EXP	DBL_MAX_10_EXP
#endif

  char *buf = __alloca ((prec > LDBL_DIG ? prec : LDBL_DIG) +
			LDBL_MAX_10_EXP + 3); /* Dot, e, exp. sign.  */
  register char *bp = buf;
#define	put(c)	*bp++ = (c)


  /* Produce the next digit in DIGIT.
     Return nonzero if it is the last.  */
  inline int hack_digit (void)
    {
      int cnt;
      mp_limb high_qlimb;

      --k;
      cy = __mpn_mul_1 (r10, r, rsize, 10);
      r10size = rsize;
      if (cy != 0)
	r10[r10size++] = cy;

      MPN_DUMP ("r", r, rsize);
      MPN_DUMP ("r10", r10, r10size);
      MPN_DUMP ("scale", scale, scalesize);

      /* Compute tmp = R10 / scale  and  R10 = R10 % scale.  */
      count_leading_zeros (cnt, scale[scalesize - 1]);
      if (cnt != 0)
	{
	  mp_limb norm_scale[scalesize];
	  mp_limb cy;
	  assert (scalesize != 0);
	  __mpn_lshift (norm_scale, scale, scalesize, cnt);
	  assert (r10size != 0);
	  cy = __mpn_lshift (r10, r10, r10size, cnt);
	  if (cy != 0)
	    r10[r10size++] = cy;
	  high_qlimb = __mpn_divmod (tmp, r10, r10size, norm_scale, scalesize);
	  tmp[r10size - scalesize] = high_qlimb;
	  r10size = scalesize;
	  __mpn_rshift (r10, r10, r10size, cnt);
	}
      else
	{
	  high_qlimb = __mpn_divmod (tmp, r10, r10size, scale, scalesize);
	  tmp[r10size - scalesize] = high_qlimb;
	  r10size = scalesize;
	}

      MPN_DUMP ("high_qlimb", &high_qlimb, 1);
      MPN_DUMP ("r10", r10, r10size);

      /* We should have a quotient < 10.  It might be stored */
      high_qlimb = tmp[0];
      digit = '0' + high_qlimb;

      r10size = __mpn_normal_size (r10, r10size);
      if (r10size == 0)
	/* We are not prepared for an mpn variable with zero limbs.  */
	r10size = 1;

      MPN_ASSIGN (r, r10);
      assert (rsize != 0);
      cy = __mpn_lshift (r2, r, rsize, 1);
      r2size = rsize;
      if (cy != 0)
	r2[r2size++] = cy;

      cy = __mpn_mul_1 (loerr, loerr, loerrsize, 10);
      if (cy)
	loerr[loerrsize++] = cy;
      cy = __mpn_mul_1 (hierr, hierr, hierrsize, 10);
      if (cy)
	hierr[hierrsize++] = cy;

      low = MPN_LT (r2, loerr);

      /* tmp = scale2 - hierr; */
      if (scale2size < hierrsize)
	high = 1;
      else
	{
	  cy = __mpn_sub (tmp, scale2, scale2size, hierr, hierrsize);
	  tmpsize = scale2size;
	  high = cy || (roundup ? MPN_GE (r2, tmp) : MPN_GT (r2, tmp));
	}

      if (low || high || k == cutoff)
	{
	  /* This is confusing, since the text and the code in Steele's and
	     White's paper are contradictory.  Problem numbers:
	     printf("%20.15e\n", <1/2^106>) is printed as
	     1.232595164407830e-32 (instead of 1.232595164407831e-32)
	     if we obey the description in the text;
	     1/2^330 is badly misprinted if we obey the code.  */
	  if (high && !low)
	    ++digit;
#define OBEY_TEXT 1
#if OBEY_TEXT
	  else if (high && low && MPN_GT (r2, scale))
#else
	  else if (high == low && MPN_GT (r2, scale))
#endif
	    ++digit;
	  return 1;
	}

      return 0;
    }

  const char *special = NULL;	/* "NaN" or "Inf" for the special cases.  */

  /* Fetch the argument value.  */
  if (info->is_long_double)
    fpnum = va_arg (*args, LONG_DOUBLE);
  else
    fpnum = (LONG_DOUBLE) va_arg (*args, double);

  /* Check for special values: not a number or infinity.  */

  if (__isnan ((double) fpnum))
    {
      special = "NaN";
      is_neg = 0;
    }
  else if (__isinf ((double) fpnum))
    {
      special = "Inf";
      is_neg = fpnum < 0;
    }

  if (special)
    {
      int width = info->prec > info->width ? info->prec : info->width;

      if (is_neg || info->showsign || info->space)
	--width;
      width -= 3;

      if (!info->left)
	while (width-- > 0)
	  outchar (' ');

      if (is_neg)
	outchar ('-');
      else if (info->showsign)
	outchar ('+');
      else if (info->space)
	outchar (' ');

      {
	register size_t len = 3;
	while (len-- > 0)
	  outchar (*special++);
      }

      if (info->left)
	while (width-- > 0)
	  outchar (' ');

      return done;
    }

  /* Split the number into a fraction and base-2 exponent.  The fractional
     part is scaled by the highest possible number of significant bits of
     fraction.  We represent the fractional part as a (very) large integer. */

  fsize = __mpn_extract_double (f, sizeof (f) / sizeof (f[0]),
				&e, &is_neg, fpnum);

  if (prec == -1)
    prec = 6;
  else if (prec == 0 && type == 'g')
    prec = 1;

  if (type == 'g')
    {
      if (fpnum != 0)
	{
	  if (is_neg)
	    fpnum = - fpnum;

	  if (fpnum < 1e-4)
	    type = 'e';
	  else
	    {			/* XXX do this more efficiently */
	      /* Is (int) floor (log10 (FPNUM)) >= PREC?  */
	      LONG_DOUBLE power = 10;
	      j = prec;
	      if (j > p)
		j = p;
	      while (--j > 0)
		{
		  power *= 10;
		  if (fpnum < power)
		    /* log10 (POWER) == floor (log10 (FPNUM)) + 1
		       log10 (FPNUM) == Number of iterations minus one.  */
		    break;
		}
	      if (j <= 0)
		/* We got all the way through the loop and F (i.e., 10**J)
		   never reached FPNUM, so we want to use %e format.  */
		type = 'e';
	    }
	}

      /* For 'g'/'G' format, the precision specifies "significant digits",
	 not digits to come after the decimal point.  */
      --prec;
    }

  if (fsize == 1 && f[0] == 0)
    /* Special case for zero.
       The general algorithm does not work for zero.  */
    {
      put ('0');
      if (tolower (info->spec) != 'g' || info->alt)
	{
	  if (prec > 0 || info->alt)
	    put (*decimal);
	  while (prec-- > 0)
	    put ('0');
	}
      if (type == 'e')
	{
	  put (info->spec);
	  put ('+');
	  put ('0');
	  put ('0');
	}
    }
  else
    {
      cutoff = -prec;

      roundup = 0;

      if (e > p)
	{
	  /* The exponent is bigger than the number of fractional digits.  */
	  MPN_ZERO (r, (e - p) / BITS_PER_MP_LIMB);
	  if ((e - p) % BITS_PER_MP_LIMB == 0)
	    {
	      MPN_COPY (r + (e - p) / BITS_PER_MP_LIMB, f, fsize);
	      rsize = fsize + (e - p) / BITS_PER_MP_LIMB;
	      assert (rsize != 0);
	    }
	  else
	    {
	      assert (fsize != 0);
	      cy = __mpn_lshift (r + (e - p) / BITS_PER_MP_LIMB, f, fsize,
				 (e - p) % BITS_PER_MP_LIMB);
	      rsize = fsize + (e - p) / BITS_PER_MP_LIMB;
	      if (cy)
		r[rsize++] = cy;
	    }

	  MPN_POW2 (scale, 0);
	  assert (scalesize != 0);

	  /* The number is (E - P) factors of two larger than
	     the fraction can represent; this is the potential error.  */
	  MPN_POW2 (loerr, e - p);
	  assert (loerrsize != 0);
	}
      else
	{
	  /* The number of fractional digits is greater than the exponent.
	     Scale by the difference factors of two.  */
	  MPN_ASSIGN (r, f);
	  MPN_POW2 (scale, p - e);
	  MPN_POW2 (loerr, 0);
	}
      MPN_ASSIGN (hierr, loerr);

      /* Fixup.  */

      MPN_POW2 (tmp, p - 1);

      if (MPN_EQ (f, tmp))
	{
	  /* Account for unequal gaps.  */
	  assert (hierrsize != 0);
	  cy = __mpn_lshift (hierr, hierr, hierrsize, 1);
	  if (cy)
	    hierr[hierrsize++] = cy;

	  assert (rsize != 0);
	  cy = __mpn_lshift (r, r, rsize, 1);
	  if (cy)
	    r[rsize++] = cy;

	  assert (scalesize != 0);
	  cy = __mpn_lshift (scale, scale, scalesize, 1);
	  if (cy)
	    scale[scalesize++] = cy;
	}

      /* scale10 = ceil (scale / 10.0).  */
      if (__mpn_divmod_1 (scale10, scale, scalesize, 10) != 0)
	{
	  /* We got a remainder.  __mpn_divmod_1 has floor'ed the quotient
	     but we want it to be ceil'ed.  Adjust.  */
	  cy = __mpn_add_1 (scale10, scale10, scalesize, 1);
	  if (cy)
	    abort ();
	}
      scale10size = scalesize;
      scale10size -= scale10[scale10size - 1] == 0;

      k = 0;
      while (MPN_LT (r, scale10))
	{
	  mp_limb cy;

	  --k;

	  cy = __mpn_mul_1 (r, r, rsize, 10);
	  if (cy != 0)
	    r[rsize++] = cy;

	  cy = __mpn_mul_1 (loerr, loerr, loerrsize, 10);
	  if (cy != 0)
	    loerr[loerrsize++] = cy;

	  cy = __mpn_mul_1 (hierr, hierr, hierrsize, 10);
	  if (cy != 0)
	    hierr[hierrsize++] = cy;
	}

      do
	{
	  mp_limb cy;
	  assert (rsize != 0);
	  cy = __mpn_lshift (r2, r, rsize, 1);
	  r2size = rsize;
	  if (cy != 0)
	    r2[r2size++] = cy;

	  /* tmp = r2 + hierr; */
	  if (r2size > hierrsize)
	    {
	      cy = __mpn_add (tmp, r2, r2size, hierr, hierrsize);
	      tmpsize = r2size;
	    }
	  else
	    {
	      cy = __mpn_add (tmp, hierr, hierrsize, r2, r2size);
	      tmpsize = hierrsize;
	    }
	  if (cy != 0)
	    tmp[tmpsize++] = cy;

	  /* while (r2 + hierr >= 2 * scale) */
	  assert (scalesize != 0);
	  cy = __mpn_lshift (scale2, scale, scalesize, 1);
	  scale2size = scalesize;
	  if (cy)
	    scale2[scale2size++] = cy;
	  while (MPN_GE (tmp, scale2))
	    {
	      cy = __mpn_mul_1 (scale, scale, scalesize, 10);
	      if (cy)
		scale[scalesize++] = cy;
	      ++k;
	      assert (scalesize != 0);
	      cy = __mpn_lshift (scale2, scale, scalesize, 1);
	      scale2size = scalesize;
	      if (cy)
		scale2[scale2size++] = cy;
	    }

	  /* Perform any necessary adjustment of loerr and hierr to
	     take into account the formatting requirements.  */

	  if (type == 'e')
	    cutoff += k - 1;	/* CutOffMode == "relative".  */
	  /* Otherwise CutOffMode == "absolute".  */

	  {			/* CutOffAdjust.  */
	    int a = cutoff - k;
	    MPN_VAR (y);
	    MPN_ASSIGN (y, scale);

	    /* There is probably a better way to do this.  */

	    while (a > 0)
	      {
		cy = __mpn_mul_1 (y, y, ysize, 10);
		if (cy)
		  y[ysize++] = cy;
		--a;
	      }
	    while (a < 0)
	      {
		if (__mpn_divmod_1 (y, y, ysize, 10) != 0)
		  {
		    /* We got a remainder.  __mpn_divmod_1 has floor'ed the
		       quotient but we want it to be ceil'ed.  Adjust.  */
		    cy = __mpn_add_1 (y, y, ysize, 1);
		    if (cy)
		      abort ();
		  }
		ysize -= y[ysize - 1] == 0;
		++a;
	      }

	    if (MPN_GT (y, loerr))
	      MPN_ASSIGN (loerr, y);
	    if (MPN_GE (y, hierr))
	      {
		MPN_ASSIGN (hierr, y);
		roundup = 1;
		/* Recalculate: tmp = r2 + hierr */
		if (r2size > hierrsize)
		  {
		    cy = __mpn_add (tmp, r2, r2size, hierr, hierrsize);
		    tmpsize = r2size;
		  }
		else
		  {
		    cy = __mpn_add (tmp, hierr, hierrsize, r2, r2size);
		    tmpsize = hierrsize;
		  }
		if (cy != 0)
		  tmp[tmpsize++] = cy;
	      }
	  }			/* End CutOffAdjust.  */

	} while (MPN_GE (tmp, scale2));

      /* End Fixup.  */

      /* First digit.  */

      hack_digit ();

      if (type == 'e')
	{
	  /* Exponential notation.  */

	  int expt = k;		/* Base-10 exponent.  */
	  int expt_neg;

	  expt_neg = k < 0;
	  if (expt_neg)
	    expt = - expt;

	  /* Find the magnitude of the exponent.  */
	  j = 10;
	  while (j <= expt)
	    j *= 10;

	  /* Write the first digit.  */
	  put (digit);

	  if (low || high || k == cutoff)
	    {
	      if ((tolower (info->spec) != 'g' && prec > 0) || info->alt)
		put (*decimal);
	    }
	  else
	    {
	      int stop;

	      put (*decimal);

	      /* Remaining digits.  */
	      do
		{
		  stop = hack_digit ();
		  put (digit);
		} while (! stop);
	    }

	  if (tolower (info->spec) != 'g' || info->alt)
	    /* Pad with zeros.  */
	    while (--k >= cutoff)
	      put ('0');

	  /* Write the exponent.  */
	  put (isupper (info->spec) ? 'E' : 'e');
	  put (expt_neg ? '-' : '+');
	  if (expt < 10)
	    /* Exponent always has at least two digits.  */
	    put ('0');
	  do
	    {
	      j /= 10;
	      put ('0' + (expt / j));
	      expt %= j;
	    }
	  while (j > 1);
	}
      else
	{
	  /* Decimal fraction notation.  */

	  if (k < 0)
	    {
	      put ('0');
	      if (prec > 0 || info->alt)
		put (*decimal);

	      /* Write leading fractional zeros.  */
	      j = 0;
	      while (--j > k)
		put ('0');
	    }

	  put (digit);
	  if (!low && !high && k != cutoff)
	    {
	      int stop;
	      do
		{
		  stop = hack_digit ();
		  if (k == -1)
		    put (*decimal);
		  put (digit);
		} while (! stop);
	    }

	  while (k > 0)
	    {
	      put ('0');
	      --k;
	    }
	  if ((type != 'g' && prec > 0) || info->alt)
	    {
	      if (k == 0)
		put (*decimal);
	      while (k-- > -prec)
		put ('0');
	    }
	}
    }

#undef	put

  /* The number is all converted in BUF.
     Now write it with sign and appropriate padding.  */

  if (is_neg || info->showsign || info->space)
    --width;

  width -= bp - buf;

  if (!info->left && info->pad == ' ')
    /* Pad with spaces on the left.  */
    while (width-- > 0)
      outchar (' ');

  /* Write the sign.  */
  if (is_neg)
    outchar ('-');
  else if (info->showsign)
    outchar ('+');
  else if (info->space)
    outchar (' ');

  if (!info->left && info->pad == '0')
    /* Pad with zeros on the left.  */
    while (width-- > 0)
      outchar ('0');

  if (fwrite (buf, bp - buf, 1, s) != 1)
    return -1;
  done += bp - buf;

  if (info->left)
    /* Pad with spaces on the right.  */
    while (width-- > 0)
      outchar (' ');

  return done;
}

#ifndef NDEBUG
static void
mpn_dump (str, p, size)
     const char *str;
     mp_limb *p;
     mp_size_t size;
{
  fprintf (stderr, "%s = ", str);
  while (size != 0)
    {
      size--;
      fprintf (stderr, "%08lX", p[size]);
    }
  fprintf (stderr, "\n");
}
#endif
