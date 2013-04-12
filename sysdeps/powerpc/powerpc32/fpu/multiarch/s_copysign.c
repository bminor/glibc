/* Multiple versions of s_copysign.
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

#include <math.h>
#include <math_ldbl_opt.h>
#include <shlib-compat.h>
#include "init-arch.h"

extern __typeof (__copysign) __copysign_ppc32 attribute_hidden;
extern __typeof (__copysign) __copysign_power6 attribute_hidden;

libc_ifunc (__copysign,
	    (hwcap & PPC_FEATURE_ARCH_2_05)
	    ? __copysign_power6
            : __copysign_ppc32);

weak_alias (__copysign, copysign)

/* It's safe to use double-precision implementation for single-precision.
   The following assembly directives are basically doing:
   weak_alias (__copysign,copysignf)
   strong_alias(__copysign,__copysignf)
   But the compiler will not let us do it in C because both __copysignf and
   copysignf are already defined.  */
asm (".weak copysignf\n"
     ".set  copysignf, __copysign");
asm (".globl __copysign\n"
     "__copysign = __copysignf");

#ifdef NO_LONG_DOUBLE
weak_alias (__copysign,copysignl)
strong_alias(__copysign,__copysignl)
#endif
#ifdef IS_IN_libm
# if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __copysign, copysignl, GLIBC_2_0);
# endif
#elif LONG_DOUBLE_COMPAT(libc, GLIBC_2_0)
compat_symbol (libc, __copysign, copysignl, GLIBC_2_0);
#endif
