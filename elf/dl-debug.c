/* Communicate dynamic linker state to the debugger at runtime.
   Copyright (C) 1996-2024 Free Software Foundation, Inc.
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


/* These are the members in the public `struct link_map' type.
   Sanity check that the internal type and the public type match.  */
#define VERIFY_MEMBER(name) \
  (offsetof (struct link_map_public, name) == offsetof (struct link_map, name))
extern const int verify_link_map_members[(VERIFY_MEMBER (l_addr)
					  && VERIFY_MEMBER (l_name)
					  && VERIFY_MEMBER (l_ld)
					  && VERIFY_MEMBER (l_next)
					  && VERIFY_MEMBER (l_prev))
					 ? 1 : -1];

#ifdef SHARED
/* r_debug structs for secondary namespaces.  The first namespace is
   handled separately because its r_debug structure must overlap with
   the public _r_debug symbol, so the first array element corresponds
   to LM_ID_BASE + 1.  See elf/dl-debug-symbols.S.  */
struct r_debug_extended _r_debug_array[DL_NNS - 1];

/* Return the r_debug object for the namespace NS.  */
static inline struct r_debug_extended *
get_rdebug (Lmid_t ns)
{
  if (ns == LM_ID_BASE)
    return &_r_debug_extended;
  else
    return  &_r_debug_array[ns - 1];
}
#else /* !SHARED */
static inline struct r_debug_extended *
get_rdebug (Lmid_t ns)
{
  return &_r_debug_extended; /* There is just one namespace.  */
}
#endif  /* !SHARED */

/* Update the `r_map' member and return the address of `struct r_debug'
   of the namespace NS. */

struct r_debug *
_dl_debug_update (Lmid_t ns)
{
  struct r_debug_extended *r = get_rdebug (ns);
  if (r->base.r_map == NULL)
    atomic_store_release (&r->base.r_map,
			  (void *) GL(dl_ns)[ns]._ns_loaded);
  return &r->base;
}

void
_dl_debug_change_state (struct r_debug *r, int state)
{
  atomic_store_release (&r->r_state, state);
  _dl_debug_state ();
}

/* Initialize _r_debug_extended for the namespace NS.  LDBASE is the
   run-time load address of the dynamic linker, to be put in
   _r_debug_extended.r_ldbase.  Return the address of _r_debug.  */

struct r_debug *
_dl_debug_initialize (ElfW(Addr) ldbase, Lmid_t ns)
{
  struct r_debug_extended *r = get_rdebug (ns);
  if (r->base.r_brk == 0)
    {
      /* Tell the debugger where to find the map of loaded objects.
	 This function is called from dlopen.  Initialize the namespace
	 only once.  */
      r->base.r_ldbase = ldbase ?: _r_debug_extended.base.r_ldbase;
      r->base.r_brk = (ElfW(Addr)) &_dl_debug_state;

#ifdef SHARED
      /* Add the new namespace to the linked list.  This assumes that
	 namespaces are allocated in increasing order.  After a
	 namespace is initialized, r_brk becomes non-zero.  A
	 namespace becomes empty (r_map == NULL) when it is unused.
	 But it is never removed from the linked list.  */

      if (ns != LM_ID_BASE)
	{
	  r->base.r_version = 2;
	  if (ns - 1 == LM_ID_BASE)
	    {
	      atomic_store_release (&_r_debug_extended.r_next, r);
	      /* Now there are multiple namespaces.  */
	      atomic_store_release (&_r_debug_extended.base.r_version, 2);
	    }
	  else
	    /* Update r_debug_extended of the previous namespace.  */
	    atomic_store_release (&_r_debug_array[ns - 2].r_next, r);
	}
      else
#endif /* SHARED */
	r->base.r_version = 1;
    }

  if (r->base.r_map == NULL)
    atomic_store_release (&r->base.r_map,
			  (void *) GL(dl_ns)[ns]._ns_loaded);

  return &r->base;
}


/* This function exists solely to have a breakpoint set on it by the
   debugger.  The debugger is supposed to find this function's address by
   examining the r_brk member of struct r_debug, but GDB 4.15 in fact looks
   for this particular symbol name in the PT_INTERP file.  */
void
_dl_debug_state (void)
{
}
rtld_hidden_def (_dl_debug_state)
