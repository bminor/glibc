/* ELF symbol resolve functions for VDSO objects.
   Copyright (C) 2005-2020 Free Software Foundation, Inc.
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

#ifndef _DL_VDSO_H
#define _DL_VDSO_H	1

#include <ldsodefs.h>
#include <dl-hash.h>

/* Functions for resolving symbols in the VDSO link map.  */
extern void *_dl_vdso_vsym (const char *name,
			    const struct r_found_version *version)
      attribute_hidden;

/* If the architecture support vDSO it should define which is the expected
   kernel version and hash value through both VDSO_NAME and VDSO_HASH
   (usually defined at architecture sysdep.h).  */

#ifndef VDSO_NAME
# define VDSO_NAME "LINUX_0.0"
#endif
#ifndef VDSO_HASH
# define VDSO_HASH 0
#endif

static inline void *
get_vdso_symbol (const char *symbol)
{
  struct r_found_version rfv = { VDSO_NAME, VDSO_HASH, 1, NULL };
  return _dl_vdso_vsym (symbol, &rfv);
}

static inline void *
get_vdso_mangle_symbol (const char *symbol)
{
  void *vdsop = get_vdso_symbol (symbol);
  PTR_MANGLE (vdsop);
  return vdsop;
}

#endif /* dl-vdso.h */
