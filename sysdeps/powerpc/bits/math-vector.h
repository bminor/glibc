/* Platform-specific SIMD declarations of math functions.
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
   <http://www.gnu.org/licenses/>.  */

#ifndef _MATH_H
# error "Never include <bits/math-vector.h> directly;\
 include <math.h> instead."
#endif

/* Get default empty definitions for simd declarations.  */
#include <bits/libm-simd-decl-stubs.h>

#if defined __PPC64__ && defined __FAST_MATH__
# if defined _OPENMP && _OPENMP >= 201307
/* OpenMP case.  */
#  define __DECL_SIMD_PPC64 _Pragma ("omp declare simd notinbranch")
# elif __GNUC_PREREQ (6,0)
/* W/o OpenMP use GCC 6.* __attribute__ ((__simd__)).  */
#  define __DECL_SIMD_PPC64 __attribute__ ((__simd__ ("notinbranch")))
# endif

# ifdef __DECL_SIMD_PPC64
#  undef __DECL_SIMD_cos
#  define __DECL_SIMD_cos __DECL_SIMD_PPC64

# endif
#endif
