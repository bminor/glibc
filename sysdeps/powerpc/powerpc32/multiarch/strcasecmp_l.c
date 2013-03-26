/* Multiple versions of strcasecmp.
   Copyright (C) 2013 Free Software Foundation, Inc.
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

#ifndef NOT_IN_libc
# ifdef SHARED
#  undef libc_hidden_builtin_def
#  define libc_hidden_builtin_def(name) \
  __hidden_ver1 (__strcasecmp_l_ppc32, __GI_strcasecmp_l, __strcasecmp_l_ppc32);
# endif

/* Redefine memmove so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef strcasecmp_l
# define strcasecmp_l __redirect_strcasecmp_l
# include <string.h>
# undef strcasecmp_l
# define strcasecmp_l __strcasecmp_l_ppc32

extern __typeof (__redirect_strcasecmp_l) __strcasecmp_l_ppc32 attribute_hidden;
extern __typeof (__redirect_strcasecmp_l) __strcasecmp_l_power7 attribute_hidden;
#endif

#include "string/strcasecmp_l.c"
#undef strcasecmp_l

#ifndef NOT_IN_libc
# include <shlib-compat.h>
# include "init-arch.h"

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_strcasecmp_l) __libc_strcasecmp_l;
libc_ifunc (__libc_strcasecmp_l,
	    (hwcap & PPC_FEATURE_HAS_VSX)
            ? __strcasecmp_l_power7
            : __strcasecmp_l_ppc32);

weak_alias (__libc_strcasecmp_l, strcasecmp_l)
#endif
