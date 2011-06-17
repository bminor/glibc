/* Bitwise manipulation over float. Function prototypes.
   Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Adhemerval Zanella <azanella@br.ibm.com>, 2011

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

#ifndef _FLOAT_BITWISE_
#define _FLOAT_BITWISE_ 1

/* Returns (num & 0x7FFFFFF0 == value) */
int __float_and_test28 (float num, float value);
/* Returns (num & 0x7FFFFF00 == value) */
int __float_and_test24 (float num, float value);
/* Returns (num & 0x7F800000) */
float __float_and8 (float num);
/* Returns ((int32_t)(num & 0x7F800000) >> 23) */
int32_t __float_get_exp (float num);
/* Returns ((num & 0x807FFFFF) | exp) */
float __float_set_exp (float num, float exp);

#endif /* s_float_bitwise.h */
