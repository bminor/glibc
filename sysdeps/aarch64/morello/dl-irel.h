/* Machine-dependent ELF indirect relocation inline functions.
   AArch64 version.
   Copyright (C) 2012-2022 Free Software Foundation, Inc.
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

#ifndef _DL_IREL_H
#define _DL_IREL_H

#include <stdio.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <sysdep.h>
#include <sys/ifunc.h>

#define ELF_MACHINE_IRELA	1

static inline uintptr_t
__attribute ((always_inline))
elf_ifunc_invoke (uintptr_t addr)
{
  __ifunc_arg_t arg;

  arg._size = sizeof (arg);
  arg._hwcap = GLRO(dl_hwcap);
  arg._hwcap2 = GLRO(dl_hwcap2);
  return ((uintptr_t (*) (uint64_t, const __ifunc_arg_t *)) (addr))
	 (GLRO(dl_hwcap) | _IFUNC_ARG_HWCAP, &arg);
}

#include <cheri_perms.h>

static inline uintptr_t
__attribute__ ((always_inline))
morello_relative_value (uintptr_t l_addr,
			const ElfW(Rela) *reloc,
			void *reloc_addr)
{
  uint64_t *__attribute__((may_alias)) u64_reloc_addr = reloc_addr;

  /* Fragment identified by r_offset has the following information:
     | 64-bit: address | 56-bits: length | 8-bits: permissions | */
  unsigned long loc = u64_reloc_addr[0];
  unsigned long len = u64_reloc_addr[1] & ((1UL << 56) - 1);
  unsigned long perm = u64_reloc_addr[1] >> 56;
  unsigned long perm_mask = 0;
  uintptr_t value = __builtin_cheri_bounds_set_exact (l_addr + loc, len);

  value = value + reloc->r_addend;

  /* Set permissions. Permissions field encoded as:
     4 = executable, 2 = read/write, 1 = read-only.
     Mask should follow the same encoding as the ELF segment permissions.  */
  if (perm == 1)
    perm_mask = CAP_PERM_MASK_R;
  if (perm == 2)
    perm_mask = CAP_PERM_MASK_RW;
  if (perm == 4)
    perm_mask = CAP_PERM_MASK_RX;
  value = __builtin_cheri_perms_and (value, perm_mask);

  /* Seal capabilities, which provide execute permission, with MORELLO_RB.  */
  if (perm == 4)
    value = __builtin_cheri_seal_entry (value);
  return value;
}

static inline void
__attribute ((always_inline))
elf_irela (const ElfW(Rela) *reloc)
{
  const unsigned long int r_type = ELFW(R_TYPE) (reloc->r_info);

  if (__glibc_likely (r_type == MORELLO_R(IRELATIVE)))
    {
      struct link_map *main_map = GL(dl_ns)[LM_ID_BASE]._ns_loaded;
      void *reloc_addr = (void *) main_map->l_addr + reloc->r_offset;
      uintptr_t *__attribute__((may_alias)) cap_reloc_addr = reloc_addr;
      uintptr_t value
	= morello_relative_value (main_map->l_addr, reloc, reloc_addr);
      *cap_reloc_addr = elf_ifunc_invoke (value);
    }
  else
    __libc_fatal ("Unexpected reloc type in static binary.\n");
}

#endif
