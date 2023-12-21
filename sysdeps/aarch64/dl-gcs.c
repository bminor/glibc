/* AArch64 GCS functions.
   Copyright (C) 2024 Free Software Foundation, Inc.

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

#include <unistd.h>
#include <errno.h>
#include <libintl.h>
#include <ldsodefs.h>

static void
fail (struct link_map *l, const char *program)
{
  if (program)
    _dl_fatal_printf ("%s: %s: not GCS compatible\n", program, l->l_name);
  else
    _dl_signal_error (0, l->l_name, "dlopen", N_("not GCS compatible"));
}

static void
check_gcs (struct link_map *l, const char *program)
{
  if (!l->l_mach.gcs)
    {
      if (GLRO(dl_aarch64_gcs_policy) == 2 || !program)
	fail (l, program);
      if (GLRO(dl_aarch64_gcs_policy) == 1 && program)
	GL(dl_aarch64_gcs) = 0;
    }
}

/* Apply GCS policy for L and its dependencies.  */

void
_dl_gcs_check (struct link_map *l, const char *program)
{
  /* GCS is disabled.  */
  if (GL(dl_aarch64_gcs) == 0)
    return;
  /* GCS marking is ignored.  */
  if (GLRO(dl_aarch64_gcs_policy) == 0)
    return;

  check_gcs (l, program);
  for (unsigned int i = 0; i < l->l_searchlist.r_nlist; i++)
    check_gcs (l->l_initfini[i], program);
}
