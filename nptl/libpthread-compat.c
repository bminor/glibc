/* Placeholder definitions to pull in removed symbol versions.
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

#include <sys/cdefs.h>
#include <shlib-compat.h>

#ifdef SHARED
static void
attribute_compat_text_section
__attribute_used__
__libpthread_version_placeholder (void)
{
}
#endif

/* This is an unused compatibility symbol definition, to prevent ld
   from creating a weak version definition for GLIBC_2.1.2.  (__vfork
   used to be defined at that version, but it is now provided by libc,
   and there are no versions left in libpthread for that symbol
   version.)  If the ABI baseline for glibc is the GLIBC_2.2 symbol
   version or later, the placeholder symbol is not needed because
   there are plenty of other symbols which populate those later
   versions.  */
#if (SHLIB_COMPAT (libpthread, GLIBC_2_1_2, GLIBC_2_2))
compat_symbol_unique (libpthread,
		      __libpthread_version_placeholder, GLIBC_2_1_2);
#endif

#if (SHLIB_COMPAT (libpthread, GLIBC_2_2_6, GLIBC_2_3))
compat_symbol_unique (libpthread,
		      __libpthread_version_placeholder, GLIBC_2_2_6);
#endif
