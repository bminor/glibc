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
   the compiler's stdarg.h for va_list specifically.
   GCC's stdarg.h checks several other macros as well as this one, in
   order to accommodate many different C libraries, but clang's
   stdarg.h only looks for this macro.  Other compilers can reasonably
   be expected to look for this macro as well.  */
#ifndef _VA_LIST

#include <bits/types/__va_list.h>

/* Check again, __va_list.h may not have been able to avoid including
   all of stdarg.h.  */
# ifndef _VA_LIST
typedef __va_list va_list;
# endif

/* This must not be defined until _after_ possibly including stdarg.h.  */
# define _VA_LIST
#endif
