/* Using math gcc builtins instead of generic implementation.  s390/s390x version.
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

#ifndef MATH_USE_BUILTINS_S390_H
#define MATH_USE_BUILTINS_S390_H	1

#ifdef HAVE_S390_MIN_Z196_ZARCH_ASM_SUPPORT

# include <features.h> /* For __GNUC_PREREQ.  */

/* GCC emits the z196 zarch "load fp integer" instructions for these
   builtins if build with at least --march=z196 -mzarch.  Otherwise a
   function call to libc is emitted.  */
# define USE_NEARBYINT_BUILTIN 1
# define USE_NEARBYINTF_BUILTIN 1
# define USE_NEARBYINTL_BUILTIN 1

# define USE_RINT_BUILTIN 1
# define USE_RINTF_BUILTIN 1
# define USE_RINTL_BUILTIN 1

# define USE_FLOOR_BUILTIN 1
# define USE_FLOORF_BUILTIN 1
# define USE_FLOORL_BUILTIN 1

# if __GNUC_PREREQ (8, 0)
#  define USE_NEARBYINTF128_BUILTIN 1
#  define USE_RINTF128_BUILTIN 1
#  define USE_FLOORF128_BUILTIN 1
# else
#  define USE_NEARBYINTF128_BUILTIN 0
#  define USE_RINTF128_BUILTIN 0
#  define USE_FLOORF128_BUILTIN 0
# endif

#else

/* Disable the builtins if we do not have the z196 zarch instructions.  */
# define USE_NEARBYINT_BUILTIN 0
# define USE_NEARBYINTF_BUILTIN 0
# define USE_NEARBYINTL_BUILTIN 0
# define USE_NEARBYINTF128_BUILTIN 0

# define USE_RINT_BUILTIN 0
# define USE_RINTF_BUILTIN 0
# define USE_RINTL_BUILTIN 0
# define USE_RINTF128_BUILTIN 0

# define USE_FLOOR_BUILTIN 0
# define USE_FLOORF_BUILTIN 0
# define USE_FLOORL_BUILTIN 0
# define USE_FLOORF128_BUILTIN 0

#endif /* ! HAVE_S390_MIN_Z196_ZARCH_ASM_SUPPORT  */

#endif /* math-use-builtins.h */
