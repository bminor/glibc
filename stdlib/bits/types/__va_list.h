/* Copyright (C) 2019 Free Software Foundation, Inc.
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

#ifndef ____va_list_defined

/* If __builtin_va_list is available, use it.  There is no predefined
   macro advertising the availability of this type.  It is known to be
   available in GCC 3.0 and later.  It is also known to be available
   in all released versions of clang.  */
#if defined __clang__ || (defined __GNUC__ && __GNUC__ >= 3)

typedef __builtin_va_list __va_list;

#else

/* Depending on the compiler, we may be able to persuade its stdarg.h
   to define an implementation-namespace alias for va_list and nothing
   else.  If this feature is not available, exposing everything
   defined by stdarg.h is better than not defining __va_list at all.  */
# define __need___va_list
# include <stdarg.h>
# undef __need___va_list

# ifdef __GNUC_VA_LIST
typedef __gnuc_va_list __va_list;
# else
typedef va_list __va_list;
# endif
#endif

/* This must not be defined until _after_ possibly including stdarg.h.  */
#define ____va_list_defined 1
#endif
