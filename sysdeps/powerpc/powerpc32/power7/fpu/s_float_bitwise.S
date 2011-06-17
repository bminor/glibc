/* Bitwise manipulation over float. PowerPC32 version using VSX.
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
   License a4byte with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 1 Franklin Street, Fifth Floor, Boston MA
   02110-1301 USA. */

#include <sysdep.h>

    .section    .rodata.cst4,"aM",@progbits,8
    .align  2
.LC0: /* 28-bits mask extended to double */
    .4byte 0x7ffffffe
    .4byte 0x00000000
.LC1: /* 24-bits mask extended to double */
    .4byte 0x7fffffe0
    .4byte 0x00000000
.LC2: /* 9-bits mask extended to double */
    .4byte 0x7ff00000
    .4byte 0x00000000
.LC3: /* mask used to get the float exp */
    .4byte 0x800FFFFF
    .4byte 0xFFFFFFFF

    .section    ".text"
    .machine    power7
/* int [r3] __float_and_test28(float [f1] num, float [f2] value)
   Returns (num & 0x7FFFFFF0 == value) */
ENTRY(__float_and_test28)
#ifdef SHARED
    mflr    r11
    cfi_register(lr,r11)
    bcl     20,31,1f
1:  mflr    r9
    addis   r9,r9,.LC0-1b@ha
    lfd     fp3,.LC0-1b@l(r9)
    mtlr    r11
    cfi_same_value (lr)
#else
    lis     r9,.LC0@ha
    lfd     fp3,.LC0@l(r9)
#endif
    xxland  v4,v1,v3
    fcmpu   cr7,fp4,fp2
    mfcr    r3,cr1
    rlwinm  r3,r3,31,1
    blr
END(__float_and_test28)

/* int [r3] __float_and_test24(float [f1] num, float [f2] value)
   Returns (num & 0x7FFFFF00 == value) */
ENTRY(__float_and_test24)
#ifdef SHARED
    mflr    r11
    cfi_register(lr,r11)
    bcl     20,31,1f
1:  mflr    r9
    addis   r9,r9,.LC1-1b@ha
    lfd     fp3,.LC1-1b@l(r9)
    mtlr    r11
    cfi_same_value (lr)
#else
    lis     r9,.LC1@ha
    lfd     fp3,.LC1@l(r9)
#endif
    xxland  v4,v1,v3
    fcmpu   cr7,fp4,fp2
    mfcr    r3,cr1
    rlwinm  r3,r3,31,1
    blr
END(__float_and_test24)

/* float [f1] __float_and8(float [f1] num)
   Returns (num & 0x7F800000) */
EALIGN(__float_and8, 4, 1)
#ifdef SHARED
    mflr    r11
    cfi_register(lr,r11)
    bcl     20,31,1f
1:  mflr    r9
    addis   r9,r9,.LC2-1b@ha
    lfd     fp3,.LC2-1b@l(r9)
    mtlr    r11
    cfi_same_value (lr)
#else
    lis     r9,.LC2@ha
    lfd     fp3,.LC2@l(r9)
#endif
    xxland  v1,v1,v3
    blr
END(__float_and8)

/* int32_t [r2] __float_get_exp(float [f1] num
   Returns ((int32_t)(num & 0x7F800000) >> 23) */
ENTRY(__float_get_exp)
#ifdef SHARED
    mflr    r11
    cfi_register(lr,r11)
    bcl     20,31,1f
1:  mflr    r9
    addis   r9,r9,.LC2-1b@ha
    lfd     fp3,.LC2-1b@l(r9)
    mtlr    r11
    cfi_same_value (lr)
#else
    lis     r9,.LC2@ha
    lfd     fp3,.LC2@l(r9)
#endif
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
#ifdef SHARED
    mflr    r11
    cfi_register(lr,r11)
    bcl     20,31,1f
1:  mflr    r9
    addis   r9,r9,.LC3-1b@ha
    lfd     fp3,.LC3-1b@l(r9)
    mtlr    r11
    cfi_same_value (lr)
#else
    lis     r9,.LC3@ha
    lfd     fp3,.LC3@l(r9)
#endif
    xxland  v4,v1,v3
    xxlor   v1,v4,v2
    blr
END(__float_set_exp)
