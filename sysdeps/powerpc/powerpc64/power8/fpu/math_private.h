/* Private inline math functions for POWER8.
   Copyright (C) 2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* The compiler isn't extracting this without going through memory, so
   we use some VSX scalar instructions to convert to the 32b format
   and move to a GPR.  */
#define GET_FLOAT_WORD(i,d)			\
  do {						\
      float tmpd = d;				\
      double tmp;				\
      long tmpi;				\
      __asm__ ("xscvdpspn %x1, %x2\n\t"		\
	       "mfvsrd %0, %x1\n\t"		\
	       : "=wr" (tmpi),			\
		 "=wa" (tmp)			\
	       : "wa" (tmpd) );			\
      i = tmpi >> 32;				\
    } while(0)

/* To ensure that we don't go through memory, we use some VSX scalar
   instructions to move VSR and to convert to the 32b format.  */
#define SET_FLOAT_WORD(d,i)		\
  do {					\
    long tmpi = i;			\
    float tmpd;				\
    tmpi = tmpi << 32;			\
    __asm__ ("mtvsrd %x0, %1\n\t"	\
	     "xscvspdpn %x0, %x0\n\t"	\
	     : "=wa" (tmpd)		\
	     : "wr" (tmpi) );		\
    d = tmpd;				\
  } while(0)

#include_next <math_private.h>
