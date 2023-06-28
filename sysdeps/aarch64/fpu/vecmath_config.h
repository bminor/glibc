/* Configuration for libmvec routines.
   Copyright (C) 2023 Free Software Foundation, Inc.
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

#ifndef _VECMATH_CONFIG_H
#define _VECMATH_CONFIG_H

#include <math_private.h>

/* Deprecated config option from Arm Optimized Routines which ensures
   fp exceptions are correctly triggered. This is not intended to be
   supported in GLIBC, however we keep it for ease of development.  */
#define WANT_SIMD_EXCEPT 0

/* Return ptr but hide its value from the compiler so accesses through it
   cannot be optimized based on the contents.  */
#define ptr_barrier(ptr)                                                      \
  ({                                                                          \
    __typeof (ptr) __ptr = (ptr);                                             \
    __asm("" : "+r"(__ptr));                                                  \
    __ptr;                                                                    \
  })

#endif
