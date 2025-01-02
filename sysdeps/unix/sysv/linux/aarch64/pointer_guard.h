/* Pointer guard implementation.  AArch64 version.
   Copyright (C) 2014-2025 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef POINTER_GUARD_H
#define POINTER_GUARD_H

/* Pointer mangling is supported for AArch64.  */
#if (IS_IN (rtld) \
     || (!defined SHARED && (IS_IN (libc) \
                             || IS_IN (libpthread))))
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(dst, src, tmp)					    \
	adrp    tmp, C_SYMBOL_NAME(__pointer_chk_guard_local);		    \
	ldr	tmp, [tmp, :lo12:C_SYMBOL_NAME(__pointer_chk_guard_local)]; \
	eor	dst, src, tmp
#  define PTR_DEMANGLE(dst, src, tmp) PTR_MANGLE (dst, src, tmp)
# else
extern uintptr_t __pointer_chk_guard_local attribute_relro attribute_hidden;
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ __pointer_chk_guard_local)
#  define PTR_DEMANGLE(var)     PTR_MANGLE (var)
# endif
#else
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(dst, src, tmp)					  \
	adrp	tmp, :got:C_SYMBOL_NAME(__pointer_chk_guard);		  \
	ldr	tmp, [tmp, :got_lo12:C_SYMBOL_NAME(__pointer_chk_guard)]; \
	ldr	tmp, [tmp];						  \
	eor	dst, src, tmp;
#  define PTR_DEMANGLE(dst, src, tmp) PTR_MANGLE (dst, src, tmp)
# else
#  include <stdint.h>
extern uintptr_t __pointer_chk_guard attribute_relro;
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ __pointer_chk_guard)
#  define PTR_DEMANGLE(var) PTR_MANGLE (var)
# endif
#endif

#endif /* POINTER_GUARD_H */
