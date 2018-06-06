/* Delayed relocation processing.  x86-64 version.
   Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

#ifndef _DL_DELAYED_RELOC_MACHINE_H
#define _DL_DELAYED_RELOC_MACHINE_H

/* This needs to be kept in sync with elf_machine_rela in
   dl-machine.h.  */
static inline void
_dl_delayed_reloc_machine (const struct dl_delayed_reloc *dr)
{
  const ElfW(Rela) *reloc = dr->reloc;
  const unsigned long int r_type = ELFW(R_TYPE) (reloc->r_info);
  ElfW(Addr) *const reloc_addr = dr->reloc_addr;
  const ElfW(Sym) *refsym = dr->refsym;
  const ElfW(Sym) *sym = dr->sym;
  ElfW(Addr) value = SYMBOL_ADDRESS (dr->sym_map, sym, true);

  if (r_type == R_X86_64_IRELATIVE)
    {
      /* Special case: IRELATIVE relocations do not have an associated
         symbol.  */
      value = dr->map->l_addr + reloc->r_addend;
      value = ((ElfW(Addr) (*) (void)) value) ();
      *reloc_addr = value;
      return;
    }

  if (ELFW(ST_TYPE) (sym->st_info) == STT_GNU_IFUNC
      && sym->st_shndx != SHN_UNDEF)
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
          strtab = (const char *) D_PTR (dr->map, l_info[DT_STRTAB]);

          _dl_error_printf (fmt, RTLD_PROGNAME, strtab + refsym->st_name);
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
    case R_X86_64_COPY:
      memcpy (reloc_addr, (void *) value,
              MIN (sym->st_size, refsym->st_size));
      break;
    default:
      _dl_reloc_bad_type (dr->map, r_type, 0);
    }
}

#endif  /* _DL_DELAYED_RELOC_MACHINE_H */
