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

/* There is no consensus among compilers as to the proper guard macro
   for having defined NULL specifically ... except NULL itself.  */
#ifndef NULL

/* In C, ((void *)0) is the preferred choice for the expansion of
   NULL, as it cannot be misinterpreted as an integer zero.  */
#ifndef __cplusplus
# define NULL ((void *)0)

/* ((void *)0) cannot be used in C++.  In C++2011 and later, nullptr
   is the preferred alternative, but programs are to be encouraged to
   migrate away from both bare 0 and NULL to nullptr, so we do not
   define NULL as nullptr.  Some compilers support an extension
   keyword __null that will trigger diagnostics when used in a context
   that expects an integer, but will also be treated as 0 for purposes
   of diagnostics encouraging migration to nullptr.

   The complexity of this #if is because clang++ always pretends to be
   G++ and may also pretend to be one of several different Windows
   compilers.  */
#elif (defined __GNUG__ || defined __clang__) \
  && !defined _MSC_VER && !defined __MINGW32__
# define NULL __null

/* Otherwise a bare 0 will have to do.  */
#else
# define NULL 0
#endif

#endif
