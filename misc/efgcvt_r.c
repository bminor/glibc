/* Double versions of reentrant *cvt_r functions.
   Copyright (C) 1995-2019 Free Software Foundation, Inc.
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

#include <efgcvt-dbl-macros.h>
#include <efgcvt_r-template.c>

#if LONG_DOUBLE_COMPAT (libc, GLIBC_2_0)
# define cvt_symbol(symbol) \
  cvt_symbol_1 (libc, __APPEND (FUNC_PREFIX, symbol), \
	      APPEND (q, symbol), GLIBC_2_0); \
  weak_alias (__APPEND (FUNC_PREFIX, symbol), APPEND (FUNC_PREFIX, symbol))
# define cvt_symbol_1(lib, local, symbol, version) \
  libc_hidden_def (local) \
  compat_symbol (lib, local, symbol, version)
#else
# define cvt_symbol(symbol) \
  cvt_symbol_1 (__APPEND (FUNC_PREFIX, symbol), APPEND (FUNC_PREFIX, symbol))
#  define cvt_symbol_1(local, symbol) \
  libc_hidden_def (local) \
  weak_alias (local, symbol)
#endif
cvt_symbol(fcvt_r);
cvt_symbol(ecvt_r);
