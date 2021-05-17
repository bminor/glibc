/* Partial initialization of ld.so loaded via static dlopen.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#include <assert.h>

/* Very special case: This object is built into the static libc, but
   must know the layout of _rtld_global_ro.  */
#define SHARED
#include <ldsodefs.h>

#include <rtld_static_init.h>

void
__rtld_static_init (struct link_map *map)
{
  const ElfW(Sym) *sym
    = _dl_lookup_direct (map, "_rtld_global_ro",
                         0x9f28436a, /* dl_new_hash output.  */
                         "GLIBC_PRIVATE",
                         0x0963cf85); /* _dl_elf_hash output.  */
  assert (sym != NULL);
  struct rtld_global_ro *dl = DL_SYMBOL_ADDRESS (map, sym);

  /* Perform partial initialization here.  Note that this runs before
     ld.so is relocated, so only members initialized without
     relocations can be written here.  */
#ifdef HAVE_AUX_VECTOR
  extern __typeof (dl->_dl_auxv) _dl_auxv attribute_hidden;
  dl->_dl_auxv = _dl_auxv;
  extern __typeof (dl->_dl_clktck) _dl_clktck attribute_hidden;
  dl->_dl_clktck = _dl_clktck;
#endif
  extern __typeof (dl->_dl_hwcap) _dl_hwcap attribute_hidden;
  dl->_dl_hwcap = _dl_hwcap;
  extern __typeof (dl->_dl_hwcap2) _dl_hwcap2 attribute_hidden;
  dl->_dl_hwcap2 = _dl_hwcap2;
  extern __typeof (dl->_dl_pagesize) _dl_pagesize attribute_hidden;
  dl->_dl_pagesize = _dl_pagesize;

  __rtld_static_init_arch (map, dl);
}
