/* Test for SME ZA state being cleared on setjmp and longjmp.
   Copyright (C) 2025 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/auxv.h>

#include <support/check.h>
#include <support/support.h>
#include <support/test-driver.h>

#include "tst-sme-helper.h"

static uint8_t *state;

static void
enable_sme_za_state (struct blk *ptr)
{
  set_tpidr2 (ptr);
  start_za ();
  load_za (state);
}

static void
check_sme_za_state (const char msg[], bool clear)
{
  unsigned long svcr = get_svcr ();
  void *tpidr2 = get_tpidr2 ();
  printf ("[%s]\n", msg);
  printf ("svcr = %016lx\n", svcr);
  printf ("tpidr2 = %016lx\n", (unsigned long)tpidr2);
  if (clear)
    {
      TEST_VERIFY (svcr == 0);
      TEST_VERIFY (tpidr2 == NULL);
    }
  else
    {
      TEST_VERIFY (svcr != 0);
      TEST_VERIFY (tpidr2 != NULL);
    }
}

static void
run (struct blk *ptr)
{
  jmp_buf buf;
  int ret;

  check_sme_za_state ("initial state", /* Clear.  */ true);

  /* Enabled ZA state so that effect of disabling be observable.  */
  enable_sme_za_state (ptr);
  check_sme_za_state ("before setjmp", /* Clear.  */ false);

  if ((ret = setjmp (buf)) == 0)
    {
      check_sme_za_state ("after setjmp", /* Clear.  */ true);

      /* Enabled ZA state so that effect of disabling be observable.  */
      enable_sme_za_state (ptr);
      check_sme_za_state ("before longjmp", /* Clear.  */ false);

      longjmp (buf, 42);

      /* Unreachable.  */
      TEST_VERIFY (false);
      __builtin_unreachable ();
    }

  TEST_COMPARE (ret, 42);
  check_sme_za_state ("after longjmp", /* Clear.  */ true);
}

static int
do_test (void)
{
  unsigned long hwcap2 = getauxval (AT_HWCAP2);
  if ((hwcap2 & HWCAP2_SME) == 0)
    return EXIT_UNSUPPORTED;

  /* Get current streaming SVE vector register size.  */
  svl = get_svl ();
  printf ("svl: %lu\n", svl);
  TEST_VERIFY_EXIT (!(svl < 16 || svl % 16 != 0 || svl >= (1 << 16)));

  /* Initialise buffer for ZA state of SME.  */
  state = xmalloc (svl * svl);
  memset (state, 1, svl * svl);
  struct blk blk = {
    .za_save_buffer = state,
    .num_za_save_slices = svl,
    .__reserved = {0},
  };

  run (&blk);

  free (state);
  return 0;
}

#include <support/test-driver.c>
