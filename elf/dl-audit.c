/* Audit common functions.
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
#include <ldsodefs.h>

void
_dl_audit_activity_map (struct link_map *l, int action)
{
  struct audit_ifaces *afct = GLRO(dl_audit);
  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
    {
      if (afct->activity != NULL)
	afct->activity (&link_map_audit_state (l, cnt)->cookie, action);
      afct = afct->next;
    }
}

void
_dl_audit_activity_nsid (Lmid_t nsid, int action)
{
  /* If head is NULL, the namespace has become empty, and the audit interface
     does not give us a way to signal LA_ACT_CONSISTENT for it because the
     first loaded module is used to identify the namespace.  */
  struct link_map *head = GL(dl_ns)[nsid]._ns_loaded;
  if (__glibc_likely (GLRO(dl_naudit) == 0)
      || head == NULL || head->l_auditing)
    return;

  _dl_audit_activity_map (head, action);
}

const char *
_dl_audit_objsearch (const char *name, struct link_map *l, unsigned int code)
{
  if (l == NULL || l->l_auditing || code == 0)
    return name;

  struct audit_ifaces *afct = GLRO(dl_audit);
  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
    {
      if (afct->objsearch != NULL)
	{
	  struct auditstate *state = link_map_audit_state (l, cnt);
	  name = afct->objsearch (name, &state->cookie, code);
	  if (name == NULL)
	    return NULL;
	}
      afct = afct->next;
   }

  return name;
}

void
_dl_audit_objopen (struct link_map *l, Lmid_t nsid)
{
  if (__glibc_likely (GLRO(dl_naudit) == 0))
    return;

  struct audit_ifaces *afct = GLRO(dl_audit);
  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
    {
      if (afct->objopen != NULL)
	{
	  struct auditstate *state = link_map_audit_state (l, cnt);
	  state->bindflags = afct->objopen (l, nsid, &state->cookie);
	  l->l_audit_any_plt |= state->bindflags != 0;
	}

      afct = afct->next;
   }
}

void
_dl_audit_objclose (struct link_map *l)
{
  if (__glibc_likely (GLRO(dl_naudit) == 0)
      || GL(dl_ns)[l->l_ns]._ns_loaded->l_auditing)
    return;

  struct audit_ifaces *afct = GLRO(dl_audit);
  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
    {
      if (afct->objclose != NULL)
	{
	  struct auditstate *state= link_map_audit_state (l, cnt);
	  /* Return value is ignored.  */
	  afct->objclose (&state->cookie);
	}

      afct = afct->next;
    }
}

void
_dl_audit_symbind_alt (struct link_map *l, const ElfW(Sym) *ref, void **value,
		       lookup_t result)
{
  if ((l->l_audit_any_plt | result->l_audit_any_plt) == 0)
    return;

  const char *strtab = (const char *) D_PTR (result, l_info[DT_STRTAB]);
  /* Compute index of the symbol entry in the symbol table of the DSO with
     the definition.  */
  unsigned int ndx = (ref - (ElfW(Sym) *) D_PTR (result, l_info[DT_SYMTAB]));

  unsigned int altvalue = 0;
  /* Synthesize a symbol record where the st_value field is the result.  */
  ElfW(Sym) sym = *ref;
  sym.st_value = (ElfW(Addr)) *value;

  struct audit_ifaces *afct = GLRO(dl_audit);
  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
    {
      struct auditstate *match_audit = link_map_audit_state (l, cnt);
      struct auditstate *result_audit = link_map_audit_state (result, cnt);
      if (afct->symbind != NULL
	  && ((match_audit->bindflags & LA_FLG_BINDFROM) != 0
	      || ((result_audit->bindflags & LA_FLG_BINDTO)
		  != 0)))
	{
	  unsigned int flags = altvalue | LA_SYMB_DLSYM;
	  uintptr_t new_value = afct->symbind (&sym, ndx,
					       &match_audit->cookie,
					       &result_audit->cookie,
					       &flags, strtab + ref->st_name);
	  if (new_value != (uintptr_t) sym.st_value)
	    {
	      altvalue = LA_SYMB_ALTVALUE;
	      sym.st_value = new_value;
	    }

	  afct = afct->next;
	}

      *value = (void *) sym.st_value;
    }
}
rtld_hidden_def (_dl_audit_symbind_alt)

void
_dl_audit_symbind (struct link_map *l, struct reloc_result *reloc_result,
		   const ElfW(Sym) *defsym, DL_FIXUP_VALUE_TYPE *value,
		   lookup_t result)
{
  reloc_result->bound = result;
  /* Compute index of the symbol entry in the symbol table of the DSO with the
     definition.  */
  reloc_result->boundndx = (defsym - (ElfW(Sym) *) D_PTR (result,
							  l_info[DT_SYMTAB]));

  if ((l->l_audit_any_plt | result->l_audit_any_plt) == 0)
    {
      /* Set all bits since this symbol binding is not interesting.  */
      reloc_result->enterexit = (1u << DL_NNS) - 1;
      return;
    }

  /* Synthesize a symbol record where the st_value field is the result.  */
  ElfW(Sym) sym = *defsym;
  sym.st_value = DL_FIXUP_VALUE_ADDR (*value);

  /* Keep track whether there is any interest in tracing the call in the lower
     two bits.  */
  assert (DL_NNS * 2 <= sizeof (reloc_result->flags) * 8);
  assert ((LA_SYMB_NOPLTENTER | LA_SYMB_NOPLTEXIT) == 3);
  reloc_result->enterexit = LA_SYMB_NOPLTENTER | LA_SYMB_NOPLTEXIT;

  const char *strtab2 = (const void *) D_PTR (result, l_info[DT_STRTAB]);

  unsigned int flags = 0;
  struct audit_ifaces *afct = GLRO(dl_audit);
  for (unsigned int cnt = 0; cnt < GLRO(dl_naudit); ++cnt)
    {
      /* XXX Check whether both DSOs must request action or only one */
      struct auditstate *l_state = link_map_audit_state (l, cnt);
      struct auditstate *result_state = link_map_audit_state (result, cnt);
      if ((l_state->bindflags & LA_FLG_BINDFROM) != 0
	  && (result_state->bindflags & LA_FLG_BINDTO) != 0)
	{
	  if (afct->symbind != NULL)
	    {
	      uintptr_t new_value = afct->symbind (&sym,
						   reloc_result->boundndx,
						   &l_state->cookie,
						   &result_state->cookie,
						   &flags,
						   strtab2 + defsym->st_name);
	      if (new_value != (uintptr_t) sym.st_value)
		{
		  flags |= LA_SYMB_ALTVALUE;
		  sym.st_value = new_value;
		}
	    }

	  /* Remember the results for every audit library and store a summary
	     in the first two bits.  */
	  reloc_result->enterexit &= flags & (LA_SYMB_NOPLTENTER
					      | LA_SYMB_NOPLTEXIT);
	  reloc_result->enterexit |= ((flags & (LA_SYMB_NOPLTENTER
						| LA_SYMB_NOPLTEXIT))
				      << ((cnt + 1) * 2));
	}
      else
	/* If the bind flags say this auditor is not interested, set the bits
	   manually.  */
	reloc_result->enterexit |= ((LA_SYMB_NOPLTENTER | LA_SYMB_NOPLTEXIT)
				    << ((cnt + 1) * 2));
      afct = afct->next;
    }

  reloc_result->flags = flags;
  *value = DL_FIXUP_ADDR_VALUE (sym.st_value);
}
