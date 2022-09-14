/* Function to access r_debug structure.  Morello version.
   Copyright (C) 2022 Free Software Foundation, Inc.
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

#include <stdint.h>
#include <link.h>

static int
E(callback) (struct dl_phdr_info *info, size_t size, void *data)
{
  /* Assume phdr covers the load segments.  */
  const void *phdr = info->dlpi_phdr;
  uintptr_t *p = data;
  unsigned long addr = *p;
  unsigned long base = __builtin_cheri_base_get (phdr);
  unsigned long len = __builtin_cheri_length_get (phdr);
  if (addr - base >= len)
    return 0;
  *p = __builtin_cheri_address_set ((uintptr_t) phdr, addr);
  return 1;
}

/* Return a valid runtime pointer to r_debug instead of an address since
   that's how this function is used despite its generic prototype.
   Assume the lookup is for the main exe and return uintptr_t type.  */

static inline uintptr_t
E(r_debug_address) (ElfW(Dyn) *d)
{
  if (d->d_tag != DT_DEBUG)
    return 0;
  uintptr_t p = d->d_un.d_ptr;
  if (dl_iterate_phdr (E(callback), &p) == 1)
    return p;
  return 0;
}

/* Return the offset of the struct r_debug before relocation.  */

static inline EW(Addr)
E(r_debug_offset) (EW(Dyn) *d, int fd, EW(Addr) offset)
{
  if (d->d_tag == DT_DEBUG)
    return (EW(Addr)) d->d_un.d_ptr;

  return 0;
}
