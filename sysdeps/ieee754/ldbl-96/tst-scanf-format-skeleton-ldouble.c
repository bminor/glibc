/* Test skeleton for formatted scanf input for ldbl-96 conversions.
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

#include <math_ldbl.h>
#include <stdint.h>
#include <string.h>

#undef compare_real
#define compare_real(x, y)						\
({									\
  ieee_long_double_shape_type ux = { .value = x }, uy = { .value = y };	\
  (ux.parts.lsw == uy.parts.lsw && ux.parts.msw == uy.parts.msw		\
   && ux.parts.sign_exponent == uy.parts.sign_exponent);	 	\
})

typedef long double type_t;

#include "tst-scanf-format-skeleton.c"
