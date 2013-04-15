/* Multiple versions of s_lrint.
   Copyright (C) 2013 Free Software Foundation, Inc.
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

/* Redefine lrintf/__lrintf so that compiler won't complain abouti
   the type mismatch with the IFUNC selector in strong_alias/weak_alias
   below.  */
#undef lrintf
#define lrintf __redirect_lrintf
#undef __lrintf
#define __lrintf __redirect___lrintf
#include <math.h>
#undef lrintf
#undef __lrintf
#include <math_ldbl_opt.h>
#include <shlib-compat.h>
#include "init-arch.h"

extern __typeof (__lrint) __lrint_ppc32 attribute_hidden;
extern __typeof (__lrint) __lrint_power6x attribute_hidden;

libc_ifunc (__lrint,
	    (hwcap & PPC_FEATURE_POWER6_EXT) ?
	      __lrint_power6x
            : __lrint_ppc32);

weak_alias (__lrint, lrint)

weak_alias (__lrint, lrintf)
strong_alias(__lrint, __lrintf)

#ifdef NO_LONG_DOUBLE
weak_alias (__lrint, lrintl)
strong_alias (__lrint, __lrintl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __lrint, lrintl, GLIBC_2_1);
#endif
