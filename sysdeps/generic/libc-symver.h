/* Symbol version management.
   Copyright (C) 1995-2021 Free Software Foundation, Inc.
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

/* This file is included from <libc-symbols.h> for !_ISOMAC, and
   unconditionally from <shlib-compat.h>.  */

#ifndef _LIBC_SYMVER_H
#define _LIBC_SYMVER_H 1

/* Use symbol_version_reference to specify the version a symbol
   reference should link to.  Use symbol_version or
   default_symbol_version for the definition of a versioned symbol.
   The difference is that the latter is a no-op in non-shared
   builds.  */
#ifdef __ASSEMBLER__
# define symbol_version_reference(real, name, version) \
     .symver real, name##@##version
#else  /* !__ASSEMBLER__ */
# define symbol_version_reference(real, name, version) \
  __asm__ (".symver " #real "," #name "@" #version)
#endif

#endif /* _LIBC_SYMVER_H */
