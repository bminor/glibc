/* IFUNC relocation processing, x86-64 version.
   Copyright (C) 2001-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>.

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

#ifndef DL_IFUNC_RELOC_H
#define DL_IFUNC_RELOC_H

static inline void
_dl_ifunc_process_relocation (const struct dl_ifunc_relocation *ifunc,
                              struct link_map *sym_map)
{
  const ElfW(Rela) *reloc = ifunc->reloc;
  const unsigned long int r_type = ELFW(R_TYPE) (reloc->r_info);
  ElfW(Addr) *const reloc_addr = ifunc->reloc_addr;

  /* Special case: A relative IFUNC relocation does not have an
     associated symbol.  */
  if (r_type == R_X86_64_IRELATIVE)
    {
      ElfW(Addr) value = sym_map->l_addr + reloc->r_addend;
      value = ((ElfW(Addr) (*) (void)) value) ();
      *reloc_addr = value;
      return;
    }

  ElfW(Addr) value = (ElfW(Addr)) sym_map->l_addr + ifunc->ifunc_sym->st_value;
  value = ((ElfW(Addr) (*) (void)) value) ();

  /* This switch statement needs to be kept in sync with the switch
     statement in elf_machine_rela.  */
  switch (r_type)
    {
    case R_X86_64_GLOB_DAT:
    case R_X86_64_JUMP_SLOT:
      *reloc_addr = value + reloc->r_addend;
      break;

    case R_X86_64_64:
      /* value + r_addend may be > 0xffffffff and R_X86_64_64
         relocation updates the whole 64-bit entry.  */
      *(Elf64_Addr *) reloc_addr = (Elf64_Addr) value + reloc->r_addend;
      break;
    case R_X86_64_32:
      value += reloc->r_addend;
      *(unsigned int *) reloc_addr = value;

      const char *fmt;
      if (__glibc_unlikely (value > UINT_MAX))
        {
          const char *strtab;

          fmt = "\
%s: Symbol `%s' causes overflow in R_X86_64_32 relocation\n";
        print_err:
          strtab = (const char *) D_PTR (sym_map, l_info[DT_STRTAB]);

          _dl_error_printf (fmt, RTLD_PROGNAME,
                            strtab + ifunc->ifunc_sym->st_name);
        }
      break;
    case R_X86_64_PC32:
      value += reloc->r_addend - (ElfW(Addr)) reloc_addr;
      *(unsigned int *) reloc_addr = value;
      if (__glibc_unlikely (value != (int) value))
        {
          fmt = "\
%s: Symbol `%s' causes overflow in R_X86_64_PC32 relocation\n";
          goto print_err;
        }
      break;
    default:
      _dl_reloc_bad_type (ifunc->reloc_map, r_type, 0);
    }
}

#endif  /* DL_IFUNC_RELOC_H */
