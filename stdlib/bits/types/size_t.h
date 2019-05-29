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

/* The guard macro for this header must match the guard macro used by
   the compiler's stddef.h for size_t specifically.
   GCC's stddef.h checks a long list of other macros as well as this
   one, in order to accommodate many different C libraries, but clang's
   stddef.h only looks for this macro.  Other compilers can reasonably
   be expected to look for this macro as well.  */
#ifndef _SIZE_T

#ifdef __SIZE_TYPE__

/* If the predefined macro __SIZE_TYPE__ is available, use it.  */
typedef __SIZE_TYPE__ size_t;

#else

/* Depending on the compiler, we may be able to persuade its stddef.h
   to define size_t and nothing else.  If this feature is not
   available, exposing everything defined by stddef.h is better than
   not defining size_t at all.  */
# define __need_size_t
# include <stddef.h>
# undef __need_size_t

#endif

/* This must not be defined until _after_ possibly including stddef.h.  */
#define _SIZE_T
#endif
