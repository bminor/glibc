/* AArch64 tests for GCS.
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

#include <support/check.h>
#include <support/support.h>
#include <support/test-driver.h>

#include <stdio.h>
#include <sys/auxv.h>

static bool __check_gcs_status (void)
{
  register unsigned long x16 asm ("x16");
  asm volatile (
    "mov	x16, #1 /* _CHKFEAT_GCS */\n"
    "hint	40 /* CHKFEAT_X16 */\n"
    : "=r" (x16));
  return x16 ^ 1;
}

static int
do_test (void)
{
  /* Check if GCS could possible by enabled.  */
  if (!(getauxval (AT_HWCAP) & HWCAP_GCS))
    {
      puts ("kernel or CPU does not support GCS");
      return EXIT_UNSUPPORTED;
    }
  bool gcs_enabled = __check_gcs_status ();
  if (gcs_enabled)
    puts ("GCS enabled");
  else
    puts ("GCS not enabled");
#if TEST_GCS_EXPECT_ENABLED
  TEST_VERIFY (gcs_enabled);
#else
  TEST_VERIFY (!gcs_enabled);
#endif
  return 0;
}

#include <support/test-driver.c>
