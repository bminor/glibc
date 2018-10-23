/* Private macros for accessing __jmp_buf contents.  ARC version.
   Copyright (C) 2006-2019 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

/* Save offsets within __jmp_buf
   We don't use most of these symbols; they are here for documentation. */

/* Callee Regs.  */
#define JB_R13 0
#define JB_R14 1
#define JB_R15 2
#define JB_R16 3
#define JB_R17 4
#define JB_R18 5
#define JB_R19 6
#define JB_R20 7
#define JB_R21 8
#define JB_R22 9
#define JB_R23 10
#define JB_R24 11
#define JB_R25 12

/* Frame Pointer, Stack Pointer, Branch-n-link.  */
#define JB_FP  13
#define JB_SP  14
#define JB_BLINK  15

/* We save space for some extra state to accommodate future changes
   This is number of words.  */
#define JB_NUM	32

/* Helper for generic ____longjmp_chk().  */
#define JB_FRAME_ADDRESS(buf) ((void *) (unsigned long int) (buf[JB_SP]))
