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

#include <ldsodefs.h>

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
