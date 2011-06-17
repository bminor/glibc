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

    .section    ".toc","aw"
.LC0:
    .tc FS_28MASK_0[TC],0x7ffffffe00000000  /* 28-bits mask */
.LC1:
    .tc FS_24MASK_0[TC],0x7fffffe000000000  /* 24-bits mask */
.LC2:
    .tc FL_09MASK_0[TC],0x7ff0000000000000  /* 9-bits mask */
.LC3:
    .tc FL_EXP_MASK[TC],0x800FFFFFFFFFFFFF  /* float exponent mask */
    .section    ".text"
    .machine    power7
/* int [r3] __float_and_test28(float [f1] num, float [f2] value)
   Returns (num & 0x7FFFFF00 == value) */
ENTRY(__float_and_test28)
    lfd     fp3,.LC0@toc(2)
    xxland  v4,v1,v3
    fcmpu   cr7,fp4,fp2
    mfcr    r3,cr1
    rlwinm  r3,r3,31,1
    blr
END(__float_and_test28)

/* int [r3] __float_and_test24(float [f1] num, float [f2] value)
   Returns (num & 0x7FFFFFF0 == value) */
ENTRY(__float_and_test24)
    lfd     fp3,.LC1@toc(2)
    xxland  v4,v1,v3
    fcmpu   cr7,fp4,fp2
    mfcr    r3,cr1
    rlwinm  r3,r3,31,1
    blr
END(__float_and_test24)

/* float [f1] __float_and8(float [f1] num)
   Returns (num & 0x7F800000) */
EALIGN(__float_and8, 4, 2)
    lfd     fp3,.LC2@toc(2)
    xxland  v1,v1,v3
    blr
END(__float_and8)

/* int32_t [r2] __float_get_exp(float [f1] num)
   Returns ((int32_t)(num & 0x7F800000) >> 23) */
ENTRY(__float_get_exp)
    lfd     fp3,.LC2@toc(2)
    xxland  v4,v1,v3
    stfs    fp4,-16(r1)
    lwz     r3,-16(r1)
    srawi   r3,r3,23
    clrldi  r3,r3,32
    blr
END(__float_get_exp)

/* float [fp1] __float_set_exp(float [fp1] num, float [fp2] exp)
   Returns ((num & 0x807FFFFF) | exp) */
EALIGN(__float_set_exp, 4, 1)
    lfd     fp3,.LC3@toc(2)
    xxland  v4,v1,v3
    xxlor   v1,v4,v2
    blr
END(__float_set_exp)
