/* Control when floating-point expressions are evaluated.  s390 version.
   Copyright (C) 2019-2020 Free Software Foundation, Inc.
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

#ifndef S390_MATH_BARRIERS_H
#define S390_MATH_BARRIERS_H 1

#ifdef HAVE_S390_VX_GCC_SUPPORT
# define ASM_CONSTRAINT_VR "v"
#else
# define ASM_CONSTRAINT_VR
#endif

#define math_opt_barrier(x)						\
  ({ __typeof (x) __x = (x);						\
    if (__builtin_types_compatible_p (__typeof (x), _Float128))		\
      __asm__ ("# math_opt_barrier_f128 %0" : "+fm" (__x));		\
    else								\
      __asm__ ("# math_opt_barrier %0"					\
	       : "+f" ASM_CONSTRAINT_VR "m" (__x));			\
    __x; })
#define math_force_eval(x)						\
  ({ __typeof (x) __x = (x);						\
    if (__builtin_types_compatible_p (__typeof (x), _Float128))		\
      __asm__ __volatile__ ("# math_force_eval_f128 %0"			\
			    : : "fm" (__x));				\
    else								\
      __asm__ __volatile__ ("# math_force_eval %0"			\
			    : : "f" ASM_CONSTRAINT_VR "m" (__x));	\
  })

#endif
