/* Post-processing of a symbol produced by dlsym, dlvsym.
   Copyright (C) 1999-2025 Free Software Foundation, Inc.
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


/* Return the link map containing the caller address.  */
static struct link_map *
_dl_sym_find_caller_link_map (ElfW(Addr) caller)
{
  struct link_map *l = _dl_find_dso_for_object (caller);
  if (l != NULL)
    return l;
  else
    /* If the address is not recognized the call comes from the main
       program (we hope).  */
    return GL(dl_ns)[LM_ID_BASE]._ns_loaded;
}

/* Translates RESULT, *REF, VALUE into a symbol address from the point
   of view of MATCH.  Performs IFUNC resolution and auditing if
   necessary.  If MATCH is NULL, CALLER is used to determine it.  */
static void *
_dl_sym_post (lookup_t result, const ElfW(Sym) *ref, void *value,
              ElfW(Addr) caller, struct link_map *match)
{
  /* Resolve indirect function address.  */
  if (__glibc_unlikely (ELFW(ST_TYPE) (ref->st_info) == STT_GNU_IFUNC))
    {
      DL_FIXUP_VALUE_TYPE fixup
        = DL_FIXUP_MAKE_VALUE (result, (ElfW(Addr)) value);
      fixup = elf_ifunc_invoke (DL_FIXUP_VALUE_ADDR (fixup));
      value = (void *) DL_FIXUP_VALUE_CODE_ADDR (fixup);
    }

#ifdef SHARED
  /* Auditing checkpoint: we have a new binding.  Provide the
     auditing libraries the possibility to change the value and
     tell us whether further auditing is wanted.  */
  if (__glibc_unlikely (GLRO(dl_naudit) > 0))
    {
      if (match == NULL)
        match = _dl_sym_find_caller_link_map (caller);
      _dl_audit_symbind_alt (match, ref, &value, result);
    }
#endif
  return value;
}
