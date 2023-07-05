/* Register Macro definitions
   Copyright (C) 2022-2023 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _SYS_REGDEF_H
#define _SYS_REGDEF_H

#define zero $r0
#define ra $r1
#define tp $r2
#define sp $r3
#define a0 $r4
#define a1 $r5
#define a2 $r6
#define a3 $r7
#define a4 $r8
#define a5 $r9
#define a6 $r10
#define a7 $r11
#define v0 $r4
#define v1 $r5
#define t0 $r12
#define t1 $r13
#define t2 $r14
#define t3 $r15
#define t4 $r16
#define t5 $r17
#define t6 $r18
#define t7 $r19
#define t8 $r20
#define x $r21
#define fp $r22
#define s0 $r23
#define s1 $r24
#define s2 $r25
#define s3 $r26
#define s4 $r27
#define s5 $r28
#define s6 $r29
#define s7 $r30
#define s8 $r31

#define fa0 $f0
#define fa1 $f1
#define fa2 $f2
#define fa3 $f3
#define fa4 $f4
#define fa5 $f5
#define fa6 $f6
#define fa7 $f7
#define fv0 $f0
#define fv1 $f1
#define ft0 $f8
#define ft1 $f9
#define ft2 $f10
#define ft3 $f11
#define ft4 $f12
#define ft5 $f13
#define ft6 $f14
#define ft7 $f15
#define ft8 $f16
#define ft9 $f17
#define ft10 $f18
#define ft11 $f19
#define ft12 $f20
#define ft13 $f21
#define ft14 $f22
#define ft15 $f23
#define fs0 $f24
#define fs1 $f25
#define fs2 $f26
#define fs3 $f27
#define fs4 $f28
#define fs5 $f29
#define fs6 $f30
#define fs7 $f31

#define vr0 $vr0
#define vr1 $vr1
#define vr2 $vr2
#define vr3 $vr3
#define vr4 $vr4
#define vr5 $vr5
#define vr6 $vr6
#define vr7 $vr7

#define xr0 $xr0
#define xr1 $xr1
#define xr2 $xr2
#define xr3 $xr3
#define xr4 $xr4
#define xr5 $xr5
#define xr6 $xr6
#define xr7 $xr7

#endif /* _SYS_REGDEF_H */
