/* Additional support macros for symbol management, used for libm.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

#ifndef _LIBM_SYMBOLS_H
#define _LIBM_SYMBOLS_H 1

#if IS_IN (libc) || IS_IN (libm)

# if defined __NO_LONG_DOUBLE_MATH && !__HAVE_DISTINCT_FLOAT128

#  define libm_hidden_mathcall(sym)		\
  libm_hidden_proto (__##sym)			\
  libm_hidden_proto (__##sym##f)

#  define libc_libm_hidden_mathcall(sym)	\
  hidden_proto (__##sym)			\
  hidden_proto (__##sym##f)

# elif !defined __NO_LONG_DOUBLE_MATH && !__HAVE_DISTINCT_FLOAT128

#  define libm_hidden_mathcall(sym)		\
  libm_hidden_proto (__##sym)			\
  libm_hidden_proto (__##sym##f)		\
  libm_hidden_proto (__##sym##l)

#  define libc_libm_hidden_mathcall(sym)	\
  hidden_proto (__##sym)			\
  hidden_proto (__##sym##f)			\
  hidden_proto (__##sym##l)

# elif defined __NO_LONG_DOUBLE_MATH && __HAVE_DISTINCT_FLOAT128

#  define libm_hidden_mathcall(sym)		\
  libm_hidden_proto (__##sym)			\
  libm_hidden_proto (__##sym##f)		\
  libm_hidden_proto (__##sym##f128)

#  define libm_hidden_mathcall(sym)		\
  hidden_proto (__##sym)			\
  hidden_proto (__##sym##f)			\
  hidden_proto (__##sym##f128)

# else /* !defined __NO_LONG_DOUBLE_MATH && __HAVE_DISTINCT_FLOAT128 */

#  define libm_hidden_mathcall(sym)		\
  libm_hidden_proto (__##sym)			\
  libm_hidden_proto (__##sym##f)		\
  libm_hidden_proto (__##sym##l)		\
  libm_hidden_proto (__##sym##f128)

#  define libc_libm_hidden_mathcall(sym)	\
  hidden_proto (__##sym)			\
  hidden_proto (__##sym##f)			\
  hidden_proto (__##sym##l)			\
  hidden_proto (__##sym##f128)

# endif
#else
# define libm_hidden_mathcall(sym)
# define libc_libm_hidden_mathcall(sym)
#endif

#if IS_IN (libm)
# define libm_attr_hidden attribute_hidden
#else
# define libm_attr_hidden /* nothing */
#endif

#endif /* libm-symbols.h */
