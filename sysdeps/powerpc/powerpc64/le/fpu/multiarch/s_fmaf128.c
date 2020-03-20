/* Multiple versions of fmaf128.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#include <libm-alias-float128.h>

#define fmaf128 __redirect_fmaf128
#include <math.h>
#undef fmaf128

#include <math_ldbl_opt.h>
#include "init-arch.h"

extern __typeof (__redirect_fmaf128) __fmaf128_ppc64 attribute_hidden;
extern __typeof (__redirect_fmaf128) __fmaf128_power9 attribute_hidden;

libc_ifunc_redirected (__redirect_fmaf128, __fmaf128,
		       (hwcap2 & PPC_FEATURE2_HAS_IEEE128)
		       ? __fmaf128_power9
		       : __fmaf128_ppc64);

libm_alias_float128 (__fma, fma)
