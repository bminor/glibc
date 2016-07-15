/* Common definitions for libm tests for _Float128.

   Copyright (C) 2017 Free Software Foundation, Inc.
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

#define FUNC(function) function ## f128
#define PRINTF_EXPR "e"
#define PRINTF_XEXPR "a"
#define PRINTF_NEXPR "f"
#define BUILD_COMPLEX(real, imag) (CMPLXF128 ((real), (imag)))
#define PREFIX FLT128
#define TYPE_STR "float128"
#define LITM(x) x ## f128
#define FTOSTR strfromf128
#define snan_value_MACRO SNANF128

/* Fixup builtins and constants for older compilers.  */
#include <features.h>
#if !__GNUC_PREREQ (7, 0)
# define LIT(x) (x ## Q)
# define FLOAT __float128
# define CFLOAT __CFLOAT128
/* The following macros are only provided since GCC 7.0.  */
# define FLT128_MANT_DIG	113
# define FLT128_DECIMAL_DIG	36
# define FLT128_DIG		33
# define FLT128_MIN_EXP		(-16381)
# define FLT128_MIN_10_EXP	(-4931)
# define FLT128_MAX_EXP		16384
# define FLT128_MAX_10_EXP	4932
# define FLT128_MAX		1.18973149535723176508575932662800702e+4932Q
# define FLT128_EPSILON		1.92592994438723585305597794258492732e-34Q
# define FLT128_MIN		3.36210314311209350626267781732175260e-4932Q
# define FLT128_TRUE_MIN	6.47517511943802511092443895822764655e-4966Q
#else
# define LIT(x) (x ## f128)
# define FLOAT _Float128
#endif

#include "test-math-floatn.h"

/* Similarly, finitef128 is not API */
extern int __finitef128 (FLOAT);

static inline int
finitef128(FLOAT f)
{
  return __finitef128 (f);
}
