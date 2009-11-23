/* Attach to target process.
   Copyright (C) 1999,2001,2002,2003,2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <version.h>

#include "thread_dbP.h"


/* Datatype for the list of known thread agents.  Normally there will
   be exactly one so we don't spend much though on making it fast.  */
LIST_HEAD (__td_agent_list);

static td_err_e
check_version (struct ps_prochandle *ps, psaddr_t versaddr)
{
  char versbuf[sizeof (VERSION)];

  if (versaddr == 0)
    return TD_NOLIBTHREAD;

  if (ps_pdread (ps, versaddr, versbuf, sizeof (versbuf)) != PS_OK)
    return TD_ERR;

  if (memcmp (versbuf, VERSION, sizeof VERSION) != 0)
    /* Not the right version.  */
    return TD_VERSION;

  return TD_OK;
}

td_err_e
td_ta_new (struct ps_prochandle *ps, td_thragent_t **ta)
{
  psaddr_t versaddr = 0;
  psaddr_t dl_versaddr = 0;

  LOG ("td_ta_new");

  /* Check whether the versions match.  */
  if (td_lookup_1 (ps, SYM_nptl_version, &versaddr, false) != PS_OK
      && td_lookup_1 (ps, SYM__thread_db_dl_nptl_version,
		      &dl_versaddr, true) != PS_OK)
    return TD_NOLIBTHREAD;

  td_err_e result = check_version (ps, versaddr ?: dl_versaddr);
  if (result != TD_OK)
    return result;

  /* Fill in the appropriate information.  */
  *ta = (td_thragent_t *) calloc (1, sizeof (td_thragent_t));
  if (*ta == NULL)
    return TD_MALLOC;

  (*ta)->ta_addr_nptl_version = versaddr;
  (*ta)->ta_addr__thread_db_dl_nptl_version = dl_versaddr;

  /* Store the proc handle which we will pass to the callback functions
     back into the debugger.  */
  (*ta)->ph = ps;

  /* Now add the new agent descriptor to the list.  */
  list_add (&(*ta)->list, &__td_agent_list);

  return TD_OK;
}

td_err_e
_td_ta_check_nptl (td_thragent_t *ta)
{
  if (ta->ta_addr_nptl_version != 0)
    return TD_OK;

  if (ta->ta_addr__thread_db_dl_nptl_version == 0)
    return TD_BADTA;

  psaddr_t versaddr = 0;
  if (td_lookup_1 (ta->ph, SYM_nptl_version, &versaddr, false) != PS_OK)
    return TD_NOLIBTHREAD;

  td_err_e result = check_version (ta->ph, versaddr);
  if (result == TD_OK)
    ta->ta_addr_nptl_version = versaddr;

  return result;
}
