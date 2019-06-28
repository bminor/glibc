/* ELF symbol resolve functions for VDSO objects.
   Copyright (C) 2005-2019 Free Software Foundation, Inc.
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

#ifndef _DL_VDSO_H
#define _DL_VDSO_H	1

#include <assert.h>
#include <ldsodefs.h>
#include <dl-hash.h>

/* Create version number record for lookup.  */
static inline struct r_found_version
prepare_version_base (const char *name, ElfW(Word) hash)
{
  assert (hash == _dl_elf_hash (name));
  return (struct r_found_version) { name, hash, 1, NULL };
}
#define prepare_version(vname) \
  prepare_version_base (VDSO_NAME_##vname, VDSO_HASH_##vname)

/* Use this for the known version sets defined below, where we
   record their precomputed hash values only once, in this file.  */
#define PREPARE_VERSION_KNOWN(var, vname) \
  struct r_found_version var = prepare_version (vname);

#define VDSO_NAME_LINUX_2_6	"LINUX_2.6"
#define VDSO_HASH_LINUX_2_6	61765110
#define VDSO_NAME_LINUX_2_6_15	"LINUX_2.6.15"
#define VDSO_HASH_LINUX_2_6_15	123718565
#define VDSO_NAME_LINUX_2_6_29	"LINUX_2.6.29"
#define VDSO_HASH_LINUX_2_6_29	123718585
#define VDSO_NAME_LINUX_2_6_39	"LINUX_2.6.39"
#define VDSO_HASH_LINUX_2_6_39	123718537
#define VDSO_NAME_LINUX_4_9	"LINUX_4.9"
#define VDSO_HASH_LINUX_4_9	61765625
#define VDSO_NAME_LINUX_4_15	"LINUX_4.15"
#define VDSO_HASH_LINUX_4_15	182943605

/* Functions for resolving symbols in the VDSO link map.  */
extern void *_dl_vdso_vsym (const char *name,
			    const struct r_found_version *version)
      attribute_hidden;

#endif /* dl-vdso.h */
