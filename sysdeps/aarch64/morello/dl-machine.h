/* Copyright (C) 1995-2022 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef dl_machine_h
#define dl_machine_h

#define ELF_MACHINE_NAME "aarch64"

#include <sysdep.h>
#include <tls.h>
#include <dl-tlsdesc.h>
#include <dl-static-tls.h>
#include <dl-irel.h>
#include <dl-machine-rel.h>
#include <cpu-features.c>

/* Translate a processor specific dynamic tag to the index in l_info array.  */
#define DT_AARCH64(x) (DT_AARCH64_##x - DT_LOPROC + DT_NUM)

/* Return nonzero iff ELF header is compatible with the running host.  */
static inline int __attribute__ ((unused))
elf_machine_matches_host (const ElfW(Ehdr) *ehdr)
{
  return ehdr->e_machine == EM_AARCH64
	 && (ehdr->e_flags & EF_AARCH64_CHERI_PURECAP) != 0;
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

static inline int __attribute__ ((unused))
elf_machine_runtime_setup (struct link_map *l, struct r_scope_elem *scope[],
			   int lazy, int profile)
{
  if (l->l_info[DT_JMPREL] && lazy)
    {
      uintptr_t *got;
      extern void _dl_runtime_resolve (ElfW(Word));
      extern void _dl_runtime_profile (ElfW(Word));

      got = (uintptr_t *) D_PTR (l, l_info[DT_PLTGOT]);
      if (got[1])
	{
	  l->l_mach.plt = (uint64_t) got[1] + l->l_addr;
	}
      got[1] = (uintptr_t) l;

      /* The got[2] entry contains the address of a function which gets
	 called to get the address of a so far unresolved function and
	 jump to it.  The profiling extension of the dynamic linker allows
	 to intercept the calls to collect information.  In this case we
	 don't store the address in the GOT so that all future calls also
	 end in this function.  */
      if ( profile)
	{
	   got[2] = (uintptr_t) &_dl_runtime_profile;

	  if (GLRO(dl_profile) != NULL
	      && _dl_name_match_p (GLRO(dl_profile), l))
	    /* Say that we really want profiling and the timers are
	       started.  */
	    GL(dl_profile_map) = l;
	}
      else
	{
	  /* This function will get called to fix up the GOT entry
	     indicated by the offset on the stack, and then jump to
	     the resolved address.  */
	  got[2] = (uintptr_t) &_dl_runtime_resolve;
	}
    }

  return lazy;
}

/* Runtime _DYNAMIC without dynamic relocations.  */
static void * __attribute__ ((unused))
elf_machine_runtime_dynamic (void)
{
  void *p;
  asm (""
    ".weak _DYNAMIC\n"
    ".hidden _DYNAMIC\n"
    "adrp %0, _DYNAMIC\n"
    "add %0, %0, :lo12:_DYNAMIC\n" : "=r"(p));
  return p;
}

/* Load address of the dynamic linker with correct bounds.  */
static uintptr_t __attribute__ ((unused))
elf_machine_load_address_from_args (void *arg)
{
  uintptr_t *sp;
  long argc;
  uintptr_t phdr = 0;
  size_t phentsize = sizeof (ElfW(Phdr));
  size_t phnum = 0;

  sp = arg;
  argc = sp[0];
  /* Skip argv.  */
  sp += argc + 2;
  /* Skip environ.  */
  for (; *sp; sp++);
  sp++;
  for (; *sp != AT_NULL; sp += 2)
    {
      long t = sp[0];
      if (t == AT_BASE && sp[1])
	return sp[1];
      else if (t == AT_PHDR)
	phdr = sp[1];
      else if (t == AT_PHNUM)
	phnum = sp[1];
      else if (t == AT_PHENT)
	phentsize = sp[1];
    }
  for (size_t i = 0; i < phnum; i++)
    {
      ElfW(Phdr) *p = (ElfW(Phdr) *)(phdr + i * phentsize);
      if (p->p_type == PT_PHDR)
	return phdr - p->p_vaddr;
      if (p->p_type == PT_DYNAMIC)
	return (uintptr_t) elf_machine_runtime_dynamic () - p->p_vaddr;
    }
  /* Fail.  */
  return 0;
}

/* In elf/rtld.c _dl_start should be global so dl-start.S can reference it.  */
#define RTLD_START asm (".globl _dl_start");

#define elf_machine_type_class(type)					\
  (((type) == MORELLO_R(JUMP_SLOT)					\
     || (type) == MORELLO_R(TPREL128)					\
     || (type) == MORELLO_R(TLSDESC)) * ELF_RTYPE_CLASS_PLT)

#define ELF_MACHINE_JMP_SLOT	MORELLO_R(JUMP_SLOT)

/* Return the address of the entry point. */
#define ELF_MACHINE_START_ADDRESS(map, start) ((map)->l_addr + ((start) - (ElfW(Addr))(map)->l_addr))

#define DL_PLATFORM_INIT dl_platform_init ()

static inline void __attribute__ ((unused))
dl_platform_init (void)
{
  if (GLRO(dl_platform) != NULL && *GLRO(dl_platform) == '\0')
    /* Avoid an empty string which would disturb us.  */
    GLRO(dl_platform) = NULL;

#ifdef SHARED
  /* init_cpu_features has been called early from __libc_start_main in
     static executable.  */
  init_cpu_features (&GLRO(dl_aarch64_cpu_features));
#endif
}


static inline uintptr_t
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
		       const ElfW(Sym) *refsym, const ElfW(Sym) *sym,
		       const ElfW(Rela) *reloc,
		       uintptr_t *reloc_addr,
		       uintptr_t value)
{
  return *reloc_addr = value;
}

/* Return the final value of a plt relocation.  */
static inline uintptr_t
elf_machine_plt_value (struct link_map *map,
		       const ElfW(Rela) *reloc,
		       uintptr_t value)
{
  return value;
}

#endif

/* Names of the architecture-specific auditing callback functions.  */
#define ARCH_LA_PLTENTER aarch64_gnu_pltenter
#define ARCH_LA_PLTEXIT  aarch64_gnu_pltexit

#ifdef RESOLVE_MAP

# include <cheri_perms.h>

static inline void
__attribute__ ((always_inline))
elf_machine_rela (struct link_map *map, struct r_scope_elem *scope[],
		  const ElfW(Rela) *reloc, const ElfW(Sym) *sym,
		  const struct r_found_version *version,
		  void *const reloc_addr, int skip_ifunc)
{
  uint64_t *__attribute__((may_alias)) u64_reloc_addr = reloc_addr;
  uintptr_t *__attribute__((may_alias)) cap_reloc_addr = reloc_addr;
  const unsigned int r_type = ELFW (R_TYPE) (reloc->r_info);

  if (r_type == MORELLO_R(RELATIVE))
    *cap_reloc_addr = morello_relative_value (map->l_addr, reloc, reloc_addr);
  else if (r_type == AARCH64_R(RELATIVE))
    *u64_reloc_addr = map->l_addr + reloc->r_addend;
  else if (__builtin_expect (r_type == R_AARCH64_NONE, 0))
    return;
  else
    {
      struct link_map *sym_map = RESOLVE_MAP (map, scope, &sym, version,
					      r_type);
      uintptr_t value = SYMBOL_ADDRESS (sym_map, sym, true);

      if (sym != NULL
	  && __glibc_unlikely (ELFW(ST_TYPE) (sym->st_info) == STT_GNU_IFUNC)
	  && __glibc_likely (sym->st_shndx != SHN_UNDEF)
	  && __glibc_likely (!skip_ifunc))
	value = elf_ifunc_invoke (value);

      switch (r_type)
	{
	case MORELLO_R(CAPINIT):
	case MORELLO_R(GLOB_DAT):
	case MORELLO_R(JUMP_SLOT):
	{
	  if (sym == NULL)
	    {
	      /* Undefined weak symbol.  */
	      *cap_reloc_addr = value + reloc->r_addend;
	      break;
	    }

	  unsigned long perm_mask;
	  switch (ELFW(ST_TYPE) (sym->st_info))
	    {
	      case STT_OBJECT:
		value = __builtin_cheri_bounds_set_exact (value, sym->st_size);
		perm_mask = CAP_PERM_MASK_RW;
		break;
	      case STT_FUNC:
	      case STT_GNU_IFUNC:
		/* Use l_addr for function pointer bounds.  */
		value = sym_map->l_addr + (value - sym_map->l_addr);
		perm_mask = CAP_PERM_MASK_RX;
		break;
	      default:
		/* STT_NONE or unknown symbol: readonly.  */
		perm_mask = CAP_PERM_MASK_R;
	    }
	  value = value + reloc->r_addend;
	  value = __builtin_cheri_perms_and (value, perm_mask);

	  /* Seal capabilities, which provide execute permission, with MORELLO_RB.  */
	  if (perm_mask == CAP_PERM_MASK_RX)
	    value = __builtin_cheri_seal_entry (value);

	  *cap_reloc_addr = value;
	}
	break;

# ifndef RTLD_BOOTSTRAP
	case AARCH64_R(ABS64):
	  *u64_reloc_addr = value + reloc->r_addend;
	  break;

	case MORELLO_R(IRELATIVE):
	{
	  uintptr_t value
	    = morello_relative_value (map->l_addr, reloc, reloc_addr);
	  if (__glibc_likely (!skip_ifunc))
	    value = elf_ifunc_invoke (value);
	  *cap_reloc_addr = value;
	}
	break;

	case MORELLO_R(TLSDESC):
	{
	  struct tlsdesc volatile *td = reloc_addr;
	  if (! sym)
	    {
	      td->pair.off = reloc->r_addend;
	      td->entry = _dl_tlsdesc_undefweak;
	    }
	  else
	    {
#  ifndef SHARED
	      CHECK_STATIC_TLS (map, sym_map);
#  else
	      if (!TRY_STATIC_TLS (map, sym_map))
		{
		  size_t size = td->pair.size;
		  if (size == 0)
		    size = sym->st_size;
		  struct tlsdesc_dynamic_arg *arg = _dl_make_tlsdesc_dynamic
		    (sym_map, sym->st_value + reloc->r_addend);
		  arg->tlsinfo.ti_size = size;
		  td->arg = arg;
		  td->entry = _dl_tlsdesc_dynamic;
		}
	      else
#  endif
		{
		  td->pair.off = sym->st_value + sym_map->l_tls_offset
				 + reloc->r_addend;
		  if (td->pair.size == 0)
		    td->pair.size = sym->st_size;
		  td->entry = _dl_tlsdesc_return;
		}
	    }
	}
	break;
	case MORELLO_R(TPREL128):
	{
	  CHECK_STATIC_TLS (map, sym_map);
	  u64_reloc_addr[0] = sym->st_value + reloc->r_addend
			      + sym_map->l_tls_offset;
	  if (u64_reloc_addr[1] == 0)
	    u64_reloc_addr[1] = sym->st_size;
	}
	break;
# endif /* !RTLD_BOOTSTRAP */
	default:
	  _dl_reloc_bad_type (map, r_type, 0);
	  break;
	}
    }
}

static inline void
__attribute__ ((always_inline))
elf_machine_rela_relative (uintptr_t l_addr,
			   const ElfW(Rela) *reloc,
			   void *const reloc_addr)
{
  uint64_t *__attribute__((may_alias)) u64_reloc_addr = reloc_addr;
  uintptr_t *__attribute__((may_alias)) cap_reloc_addr = reloc_addr;
  const unsigned int r_type = ELFW (R_TYPE) (reloc->r_info);
  if (r_type == MORELLO_R(RELATIVE))
    *cap_reloc_addr = morello_relative_value (l_addr, reloc, reloc_addr);
  else
    *u64_reloc_addr = l_addr + reloc->r_addend;
}

static inline void
__attribute__ ((always_inline))
elf_machine_lazy_rel (struct link_map *map, struct r_scope_elem *scope[],
		      uintptr_t l_addr,
		      const ElfW(Rela) *reloc,
		      int skip_ifunc)
{
  void *reloc_addr = (void *) (l_addr + reloc->r_offset);
  uintptr_t *__attribute__((may_alias)) cap_reloc_addr = reloc_addr;
  const unsigned int r_type = ELFW (R_TYPE) (reloc->r_info);
  /* Check for unexpected PLT reloc type.  */
  if (__builtin_expect (r_type == MORELLO_R(JUMP_SLOT), 1))
    {
      if (__glibc_unlikely (map->l_info[DT_AARCH64 (VARIANT_PCS)] != NULL))
	{
	  /* Check the symbol table for variant PCS symbols.  */
	  const Elf_Symndx symndx = ELFW (R_SYM) (reloc->r_info);
	  const ElfW (Sym) *symtab =
	    (const void *)D_PTR (map, l_info[DT_SYMTAB]);
	  const ElfW (Sym) *sym = &symtab[symndx];
	  if (__glibc_unlikely (sym->st_other & STO_AARCH64_VARIANT_PCS))
	    {
	      /* Avoid lazy resolution of variant PCS symbols.  */
	      const struct r_found_version *version = NULL;
	      if (map->l_info[VERSYMIDX (DT_VERSYM)] != NULL)
		{
		  const ElfW (Half) *vernum =
		    (const void *)D_PTR (map, l_info[VERSYMIDX (DT_VERSYM)]);
		  version = &map->l_versions[vernum[symndx] & 0x7fff];
		}
	      elf_machine_rela (map, scope, reloc, sym, version, reloc_addr,
				skip_ifunc);
	      return;
	    }
	}

      if (map->l_mach.plt == 0)
	*cap_reloc_addr = (uint64_t) *cap_reloc_addr + l_addr;
      else
	*cap_reloc_addr = map->l_mach.plt;
    }
  else if (__builtin_expect (r_type == MORELLO_R(TLSDESC), 1))
    {
      const Elf_Symndx symndx = ELFW (R_SYM) (reloc->r_info);
      const ElfW (Sym) *symtab = (const void *)D_PTR (map, l_info[DT_SYMTAB]);
      const ElfW (Sym) *sym = &symtab[symndx];
      const struct r_found_version *version = NULL;

      if (map->l_info[VERSYMIDX (DT_VERSYM)] != NULL)
	{
	  const ElfW (Half) *vernum =
	    (const void *)D_PTR (map, l_info[VERSYMIDX (DT_VERSYM)]);
	  version = &map->l_versions[vernum[symndx] & 0x7fff];
	}

      /* Always initialize TLS descriptors completely, because lazy
	 initialization requires synchronization at every TLS access.  */
      elf_machine_rela (map, scope, reloc, sym, version, reloc_addr,
			skip_ifunc);
    }
  else if (__glibc_unlikely (r_type == MORELLO_R(IRELATIVE)))
    {
      uintptr_t value
	= morello_relative_value (map->l_addr, reloc, reloc_addr);
      if (__glibc_likely (!skip_ifunc))
	value = elf_ifunc_invoke (value);
      *cap_reloc_addr = value;
    }
  else
    _dl_reloc_bad_type (map, r_type, 1);
}

#endif
