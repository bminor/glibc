/* Basic tests for sealing.  Static version.
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

#include <support/xthread.h>

/* This test checks the memory sealing work on a statically built binary.  */

#define TEST_STATIC              1

/* Expected libraries that loader will seal.  */
static const char *expected_sealed_vmas[] =
{
  "tst-dl_mseal-static",
};

/* Auxiliary pages mapped by the kernel.  */
static const char *expected_non_sealed_vmas[] =
{
  "[vdso]",
  "[sigpage]",
};

/* Auxiliary kernel pages where permission can not be changed.  */
static const char *expected_non_sealed_special[] =
{
  "[vectors]",
};

static void *
tf (void *closure)
{
  return NULL;
}

static void
run_extra_steps (void)
{
  xpthread_join (xpthread_create (NULL, tf, NULL));
}

#include "tst-dl_mseal-skeleton.c"
