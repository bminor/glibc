/* Call the termination functions of loaded shared objects.
   Copyright (C) 1995-2023 Free Software Foundation, Inc.
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
#include <string.h>
#include <ldsodefs.h>
#include <elf-initfini.h>

void
_dl_fini (void)
{
  /* Call destructors strictly in the reverse order of constructors.
     This causes fewer surprises than some arbitrary reordering based
     on new (relocation) dependencies.  None of the objects are
     unmapped, so applications can deal with this if their DSOs remain
     in a consistent state after destructors have run.  */

  /* Protect against concurrent loads and unloads.  */
  __rtld_lock_lock_recursive (GL(dl_load_lock));

  /* Ignore objects which are opened during shutdown.  */
  struct link_map *local_init_called_list = _dl_init_called_list;

  for (struct link_map *l = local_init_called_list; l != NULL;
       l = l->l_init_called_next)
      /* Bump l_direct_opencount of all objects so that they
	 are not dlclose()ed from underneath us.  */
      ++l->l_direct_opencount;

  /* After this point, everything linked from local_init_called_list
     cannot be unloaded because of the reference counter update.  */
  __rtld_lock_unlock_recursive (GL(dl_load_lock));

  /* Perform two passes: One for non-audit modules, one for audit
     modules.  This way, audit modules receive unload notifications
     for non-audit objects, and the destructors for audit modules
     still run.  */
#ifdef SHARED
  int last_pass = GLRO(dl_naudit) > 0;
  Lmid_t last_ns = -1;
  for (int do_audit = 0; do_audit <= last_pass; ++do_audit)
#endif
    for (struct link_map *l = local_init_called_list; l != NULL;
	 l = l->l_init_called_next)
      {
#ifdef SHARED
	if (GL(dl_ns)[l->l_ns]._ns_loaded->l_auditing != do_audit)
	  continue;

	/* Avoid back-to-back calls of _dl_audit_activity_nsid for the
	   same namespace.  */
	if (last_ns != l->l_ns)
	  {
	    if (last_ns >= 0)
	      _dl_audit_activity_nsid (last_ns, LA_ACT_CONSISTENT);
	    _dl_audit_activity_nsid (l->l_ns, LA_ACT_DELETE);
	    last_ns = l->l_ns;
	  }
#endif

	/* There is no need to re-enable exceptions because _dl_fini
	   is not called from a context where exceptions are caught.  */
	_dl_call_fini (l);

#ifdef SHARED
	/* Auditing checkpoint: another object closed.  */
	_dl_audit_objclose (l);
#endif
      }

#ifdef SHARED
  if (last_ns >= 0)
    _dl_audit_activity_nsid (last_ns, LA_ACT_CONSISTENT);

  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_STATISTICS))
    _dl_debug_printf ("\nruntime linker statistics:\n"
		      "           final number of relocations: %lu\n"
		      "final number of relocations from cache: %lu\n",
		      GL(dl_num_relocations),
		      GL(dl_num_cache_relocations));
#endif
}
