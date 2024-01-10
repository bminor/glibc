/* AArch64 GCS functions.
   Copyright (C) 2024-2025 Free Software Foundation, Inc.

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
#include <ldsodefs.h>

/* GCS is disabled.  */
#define GCS_POLICY_DISABLED 0

/* Enable GCS, abort if unmarked binary is found.  */
#define GCS_POLICY_ENFORCED 1

/* Optionally enable GCS if all startup dependencies are marked.  */
#define GCS_POLICY_OPTIONAL 2

/* Override binary marking and always enabled GCS.  */
#define GCS_POLICY_OVERRIDE 3

static void
fail (struct link_map *l, const char *program)
{
  if (program && program[0])
    _dl_fatal_printf ("%s: %s: %s\n", program, l->l_name, "not GCS compatible");
  else if (program)
    _dl_fatal_printf ("%s\n", "not GCS compatible");
  else
    _dl_signal_error (0, l->l_name, "dlopen", "not GCS compatible");
}

static void
unsupported (void)
{
  _dl_fatal_printf ("%s\n", "unsupported GCS policy");
}

/* This function is called only when binary markings are not
   ignored and GCS is supposed to be enabled.  This occurs
   for the GCS_POLICY_ENFORCED and GCS_POLICY_ENFORCED policies.  */
static bool
check_gcs (struct link_map *l, const char *program, bool enforced)
{
#ifdef SHARED
  /* Ignore GCS marking on ld.so: its properties are not processed.  */
  if (is_rtld_link_map (l->l_real))
    return true;
#endif
  /* Binary is marked, all good.  */
  if (l->l_mach.gcs)
    return true;
  /* Binary is not marked and loaded via dlopen: abort.  */
  if (program == NULL)
    fail (l, program);
  /* Binary is not marked and we enforce GCS: abort.  */
  if (enforced)
    fail (l, program);
  /* Binary is not marked but GSC is optional: disable GCS.  */
  else
    {
      GL(dl_aarch64_gcs) = 0;
      return false;
    }
  __builtin_unreachable ();
}

/* Iterate over all dependencies and check GCS marking.
   This function is called only when binary markings are not
   ignored and GCS is supposed to be enabled.  This occurs
   for the GCS_POLICY_ENFORCED and GCS_POLICY_ENFORCED policies.
   We interrupt checking if GCS is optional and we already know
   it is going to be disabled. */
static void
check_gcs_depends (struct link_map *l, const char *program, bool enforced)
{
  if (check_gcs (l, program, enforced))
    for (unsigned int i = 0; i < l->l_searchlist.r_nlist; i++)
      if (!check_gcs (l->l_searchlist.r_list[i], program, enforced))
	break;
}

/* Apply GCS policy for L and its dependencies.
   PROGRAM is NULL when this check is invoked for dl_open.  */
void
_dl_gcs_check (struct link_map *l, const char *program)
{
  unsigned long policy = GL (dl_aarch64_gcs);
  switch (policy)
    {
    case GCS_POLICY_DISABLED:
    case GCS_POLICY_OVERRIDE:
      return;
    case GCS_POLICY_ENFORCED:
      check_gcs_depends (l, program, true);
      return;
    case GCS_POLICY_OPTIONAL:
      check_gcs_depends (l, program, false);
      return;
    default:
      /* All other policy values are not supported: abort.  */
      unsupported ();
    }
}

/* Used to report error when prctl system call to enabled GCS fails.  */
void _dl_gcs_enable_failed (int code)
{
  _dl_fatal_printf ("failed to enable GCS: %d\n", -code);
}
