/* Define __FP_BUILTIN_DENORMAL.
   Copyright (C) 2025 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#ifndef _MATH_H
# error "Never use <bits/fp-builtin-denormal.h> directly; include <math.h> instead."
#endif

/* Neither GCC (bug 123161) nor clang (issue 172533) handles pseudo-normal
   numbers correctly with fpclassify builtin.  */
#define __FP_BUILTIN_FPCLASSIFY_DENORMAL 0

/* Neither GCC (bug 123173) nor clang (issue 172651) handles pseudo-normal
   numbers correctly with isinf_sign builtin.  */
#define __FP_BUILTIN_ISINF_SIGN_DENORMAL 0
