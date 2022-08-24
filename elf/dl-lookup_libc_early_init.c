/* Find the address of the __libc_early_init function.
   Copyright (C) 2020-2022 Free Software Foundation, Inc.
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

#include <ldsodefs.h>
#include <libc-early-init.h>
#include <link.h>
#include <stddef.h>

__typeof (__libc_early_init) *
_dl_lookup_libc_early_init (struct link_map *libc_map)
{
  const ElfW(Sym) *sym
    = _dl_lookup_direct (libc_map, LIBC_EARLY_INIT_NAME_STRING,
                         LIBC_EARLY_INIT_GNU_HASH,
                         "GLIBC_PRIVATE",
                         0x0963cf85); /* _dl_elf_hash output.  */
  if (sym == NULL)
    _dl_signal_error (0, libc_map->l_name, NULL, "\
ld.so/libc.so mismatch detected (upgrade in progress?)");
  return DL_SYMBOL_ADDRESS (libc_map, sym);
}
