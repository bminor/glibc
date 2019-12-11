/* Using math gcc builtins instead of generic implementation.  Generic version.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

#ifndef MATH_USE_BUILTINS_H
#define MATH_USE_BUILTINS_H	1

/* Define these macros to 1 to use __builtin_xyz instead of the
   generic implementation.  */
#define USE_NEARBYINT_BUILTIN 0
#define USE_NEARBYINTF_BUILTIN 0
#define USE_NEARBYINTL_BUILTIN 0
#define USE_NEARBYINTF128_BUILTIN 0

#define USE_RINT_BUILTIN 0
#define USE_RINTF_BUILTIN 0
#define USE_RINTL_BUILTIN 0
#define USE_RINTF128_BUILTIN 0

#endif /* math-use-builtins.h */
