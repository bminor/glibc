/* Denormal number definitions.
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

/*  __FP_BUILTIN_FPCLASSIFY_DENORMAL is defined to 1 if compiler supports
    handling pseudo-denormal numbers with fpclassify builtin.  Pseudo-denormal
    is a non-standard denormalized floating-point number only supported by
    Intel double extended-precision (long double).  By default assume 1 to
    enable the usage of compiler builtin on math.h.  */
#define __FP_BUILTIN_FPCLASSIFY_DENORMAL 1
