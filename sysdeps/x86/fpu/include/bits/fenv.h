/* Wrapper for x86 bits/fenv.h for use when building glibc.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.
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

#ifndef _BITS_FENV_H

/* The installed version of bits/fenv.h only declares
   __feraiseexcept_renamed when _LIBC is not defined.  */
#if defined __USE_EXTERN_INLINES && defined _LIBC
# if defined SHARED && !defined NO_HIDDEN && IS_IN (libm)
extern int __REDIRECT_NTH (__feraiseexcept_renamed, (int),
                           __GI_feraiseexcept) attribute_hidden;
# else
extern int __REDIRECT_NTH (__feraiseexcept_renamed, (int), feraiseexcept);
# endif
#endif

#include_next <bits/fenv.h>

#if defined __USE_EXTERN_INLINES && !defined _ISOMAC

/* Ensure __feraiseexcept calls in glibc are optimized the same as
   feraiseexcept calls.  */

__BEGIN_DECLS

__extern_inline int
__NTH (__feraiseexcept (int __excepts))
{
  if (__builtin_constant_p (__excepts)
      && (__excepts & ~(FE_INVALID | FE_DIVBYZERO)) == 0)
    {
      __feraiseexcept_invalid_divbyzero (__excepts);
      return 0;
    }

  return __feraiseexcept_renamed (__excepts);
}

__END_DECLS

#endif /* __USE_EXTERN_INLINES && !_ISOMAC */
#endif /* bits/fenv.h */
