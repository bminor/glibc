/* Copyright (C) 1996-2025 Free Software Foundation, Inc.
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

#include <aliases.h>


#define LOOKUP_TYPE		struct aliasent
#define SETFUNC_NAME		setaliasent
#define	GETFUNC_NAME		getaliasent
#define	ENDFUNC_NAME		endaliasent
#define DATABASE_NAME		aliases

/* There is no nscd support for the aliases file.  */
#undef	USE_NSCD

#include "../nss/getXXent_r.c"
