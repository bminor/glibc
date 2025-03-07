/* Read the dynamic section at DYN and fill in INFO with indices DT_*.
   Copyright (C) 2012-2025 Free Software Foundation, Inc.
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

/* Populate dynamic tags in l_info.  */

#ifndef _GET_DYNAMIC_INFO_H
#define _GET_DYNAMIC_INFO_H

#include <assert.h>
#include <dl-machine-rel.h>
#include <libc-diag.h>

static inline void __attribute__ ((unused, always_inline))
elf_get_dynamic_info (struct link_map *l, bool bootstrap,
		      bool static_pie_bootstrap)
{
#if __ELF_NATIVE_CLASS == 32
  typedef Elf32_Word d_tag_utype;
#elif __ELF_NATIVE_CLASS == 64
  typedef Elf64_Xword d_tag_utype;
#endif

#ifndef STATIC_PIE_BOOTSTRAP
  if (!bootstrap && l->l_ld == NULL)
    return;
#endif

  ElfW(Dyn) **info = l->l_info;

  for (ElfW(Dyn) *dyn = l->l_ld; dyn->d_tag != DT_NULL; dyn++)
    {
      d_tag_utype i;

      if ((d_tag_utype) dyn->d_tag < DT_NUM)
	i = dyn->d_tag;
      else if (dyn->d_tag >= DT_LOPROC
	       && dyn->d_tag < DT_LOPROC + DT_THISPROCNUM)
	i = dyn->d_tag - DT_LOPROC + DT_NUM;
      else if ((d_tag_utype) DT_VERSIONTAGIDX (dyn->d_tag) < DT_VERSIONTAGNUM)
	i = VERSYMIDX (dyn->d_tag);
      else if ((d_tag_utype) DT_EXTRATAGIDX (dyn->d_tag) < DT_EXTRANUM)
	i = DT_EXTRATAGIDX (dyn->d_tag) + DT_NUM + DT_THISPROCNUM
	    + DT_VERSIONTAGNUM;
      else if ((d_tag_utype) DT_VALTAGIDX (dyn->d_tag) < DT_VALNUM)
	i = DT_VALTAGIDX (dyn->d_tag) + DT_NUM + DT_THISPROCNUM
	    + DT_VERSIONTAGNUM + DT_EXTRANUM;
      else if ((d_tag_utype) DT_ADDRTAGIDX (dyn->d_tag) < DT_ADDRNUM)
	i = DT_ADDRTAGIDX (dyn->d_tag) + DT_NUM + DT_THISPROCNUM
	    + DT_VERSIONTAGNUM + DT_EXTRANUM + DT_VALNUM;
      else
	continue;

      info[i] = dyn;
    }

  /* Don't adjust .dynamic unnecessarily.  */
  if (l->l_addr != 0 && dl_relocate_ld (l))
    {
      ElfW(Addr) l_addr = l->l_addr;

# define ADJUST_DYN_INFO(tag) \
      do								      \
	{								      \
	  if (info[tag] != NULL)					      \
	  info[tag]->d_un.d_ptr += l_addr;				      \
	}								      \
      while (0)

      ADJUST_DYN_INFO (DT_HASH);
      ADJUST_DYN_INFO (DT_PLTGOT);
      ADJUST_DYN_INFO (DT_STRTAB);
      ADJUST_DYN_INFO (DT_SYMTAB);
      ADJUST_DYN_INFO (DT_RELR);
      ADJUST_DYN_INFO (DT_JMPREL);
      ADJUST_DYN_INFO (VERSYMIDX (DT_VERSYM));
      ADJUST_DYN_INFO (ADDRIDX (DT_GNU_HASH));
# undef ADJUST_DYN_INFO

      /* DT_RELA/DT_REL are mandatory.  But they may have zero value if
	 there is DT_RELR.  Don't relocate them if they are zero.  */
# define ADJUST_DYN_INFO(tag) \
      do								      \
	if (info[tag] != NULL && info[tag]->d_un.d_ptr != 0)		      \
         info[tag]->d_un.d_ptr += l_addr;				      \
      while (0)

# if ! ELF_MACHINE_NO_RELA
      ADJUST_DYN_INFO (DT_RELA);
# endif
# if ! ELF_MACHINE_NO_REL
      ADJUST_DYN_INFO (DT_REL);
# endif
# undef ADJUST_DYN_INFO
    }
  if (info[DT_PLTREL] != NULL)
    {
#if ELF_MACHINE_NO_RELA
      assert (info[DT_PLTREL]->d_un.d_val == DT_REL);
#elif ELF_MACHINE_NO_REL
      assert (info[DT_PLTREL]->d_un.d_val == DT_RELA);
#else
      assert (info[DT_PLTREL]->d_un.d_val == DT_REL
	      || info[DT_PLTREL]->d_un.d_val == DT_RELA);
#endif
    }
#if ! ELF_MACHINE_NO_RELA
  if (info[DT_RELA] != NULL)
    assert (info[DT_RELAENT]->d_un.d_val == sizeof (ElfW(Rela)));
# endif
# if ! ELF_MACHINE_NO_REL
  if (info[DT_REL] != NULL)
    assert (info[DT_RELENT]->d_un.d_val == sizeof (ElfW(Rel)));
#endif
  if (info[DT_RELR] != NULL)
    assert (info[DT_RELRENT]->d_un.d_val == sizeof (ElfW(Relr)));
  if (bootstrap || static_pie_bootstrap)
    {
      assert (info[DT_RUNPATH] == NULL);
      assert (info[DT_RPATH] == NULL);
    }
  if (bootstrap)
    {
      /* Only the bind now flags are allowed.  */
      assert (info[VERSYMIDX (DT_FLAGS_1)] == NULL
	      || (info[VERSYMIDX (DT_FLAGS_1)]->d_un.d_val & ~DF_1_NOW) == 0);
      /* Flags must not be set for ld.so.  */
      assert (info[DT_FLAGS] == NULL
	      || (info[DT_FLAGS]->d_un.d_val & ~DF_BIND_NOW) == 0);
    }
  else
    {
      if (info[DT_FLAGS] != NULL)
	{
	  /* Flags are used.  Translate to the old form where available.
	     Since these l_info entries are only tested for NULL pointers it
	     is ok if they point to the DT_FLAGS entry.  */
	  l->l_flags = info[DT_FLAGS]->d_un.d_val;

	  if (l->l_flags & DF_SYMBOLIC)
	    info[DT_SYMBOLIC] = info[DT_FLAGS];
	  if (l->l_flags & DF_TEXTREL)
	    info[DT_TEXTREL] = info[DT_FLAGS];
	  if (l->l_flags & DF_BIND_NOW)
	    info[DT_BIND_NOW] = info[DT_FLAGS];
	}

      if (info[VERSYMIDX (DT_FLAGS_1)] != NULL)
	{
	  l->l_flags_1 = info[VERSYMIDX (DT_FLAGS_1)]->d_un.d_val;
	  if (l->l_flags_1 & DF_1_NODELETE)
	    l->l_nodelete_pending = true;

	  /* Only DT_1_SUPPORTED_MASK bits are supported, and we would like
	     to assert this, but we can't. Users have been setting
	     unsupported DF_1_* flags for a long time and glibc has ignored
	     them. Therefore to avoid breaking existing applications the
	     best we can do is add a warning during debugging with the
	     intent of notifying the user of the problem.  */
	  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FILES, 0)
	      && l->l_flags_1 & ~DT_1_SUPPORTED_MASK)
	    _dl_debug_printf ("\nWARNING: Unsupported flag value(s) of 0x%x "
			      "in DT_FLAGS_1.\n",
			     l->l_flags_1 & ~DT_1_SUPPORTED_MASK);

	 if (l->l_flags_1 & DF_1_NOW)
	   info[DT_BIND_NOW] = info[VERSYMIDX (DT_FLAGS_1)];
       }

    if (info[DT_RUNPATH] != NULL)
      /* If both RUNPATH and RPATH are given, the latter is ignored.  */
      info[DT_RPATH] = NULL;
   }
}

#endif
