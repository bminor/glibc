/* Bitwise manipulation over float. PowerPC64 version using VSX.
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

#include <sysdep.h>

/* int [r3] __float_and_test28(float [f1] num, float [f2] value)
   Returns (num & 0x7FFFFFF0 == value) */
ENTRY(__float_and_test28)
    stfs    fp1,-32(r1)
    stfs    fp2,-16(r1)
    lwz     r0,-32(r1)
    rlwinm  r3,r0,0,0,27
    lwz     r0,-16(r1)
    xor     r3,r3,r0
    cntlzw  r3,r3
    srwi    r3,r3,5
    extsw   r3,r3
    blr
END(__float_and_test28)

/* int [r3] __float_and_test24(float [f1] num, float [f2] value)
   Returns (num & 0x7FFFFF00 == value) */
ENTRY(__float_and_test24)
    stfs    fp1,-32(r1)
    stfs    fp2,-16(r1)
    lwz     r0,-32(r1)
    rlwinm  r3,r0,0,0,23
    lwz     r0,-16(r1)
    xor     r3,r3,r0
    cntlzw  r3,r3
    srwi    r3,r3,5
    extsw   r3,r3
    blr
END(__float_and_test24)

/* float [f1] __float_and8(float [f1] num)
   Returns (num & 0x7F800000) */
ENTRY(__float_and8)
    stfs   fp1,-16(r1)
    lwz    r0,-16(r1)
    rlwinm r0,r0,0,1,8
    stw    r0,-16(r1)
    lfs    fp1,-16(r1)
    blr
END(__float_and8)

/* int32_t [r2] __float_get_exp(float [f1] num)
   Returns ((int32_t)(num & 0x7F800000) >> 23) */
ENTRY(__float_get_exp)
    stfs   fp1,-16(r1)
    lwz    r0,-16(r1)
    rlwinm r3,r0,0,1,8
    rldicl r3,r3,41,56
    blr
END(__float_get_exp)

/* float [fp1] __float_set_exp(float [fp1] num, float [fp2] exp)
   Returns ((num & 0x807FFFFF) | exp) */
ENTRY(__float_set_exp)
    stfs   fp1,-32(r1)
    stfs   fp2,-16(r1)
    lwz    r9,-32(r1)
    rlwinm r0,r9,1,8,31
    rlwinm r0,r0,31,0xffffffff
    lwz    r9,-16(r1)
    or     r9,r0,r9
    stw    r9,-16(r1)
    lfs    fp1,-16(r1)
    blr
END(__float_set_exp)
