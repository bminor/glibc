/* Multiple versions of s_llround.
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

/* Redefine llroundf/__llroundf so that compiler won't complain abouti
   the type mismatch with the IFUNC selector in strong_alias/weak_alias
   below.  */
#undef llroundf
#define llroundf __redirect_llroundf
#undef __llroundf
#define __llroundf __redirect___llroundf
#include <math.h>
#undef llroundf
#undef __llroundf
#include <math_ldbl_opt.h>
#include <shlib-compat.h>
#include "init-arch.h"

extern __typeof (__llround) __llround_ppc32 attribute_hidden;
extern __typeof (__llround) __llround_power4 attribute_hidden;
extern __typeof (__llround) __llround_power5 attribute_hidden;
extern __typeof (__llround) __llround_power6 attribute_hidden;

libc_ifunc (__llround,
	    (hwcap & PPC_FEATURE_ARCH_2_05)
	    ? __llround_power6 :
	      (hwcap & PPC_FEATURE_POWER5_PLUS)
	      ? __llround_power5 :
		(hwcap & PPC_FEATURE_POWER4)
		? __llround_power4
            : __llround_ppc32);

weak_alias (__llround, llround)

strong_alias (__llround, __llroundf)
weak_alias (__llround, llroundf)

#ifdef NO_LONG_DOUBLE
strong_alias (__llround, __llroundl)
weak_alias (__llround, llroundl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __llround, llroundl, GLIBC_2_1);
#endif
