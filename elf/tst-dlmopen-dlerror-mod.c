/* Check that dlfcn errors are reported properly after dlmopen.  Test module.
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
   <http://www.gnu.org/licenses/>.  */

#include <dlfcn.h>
#include <stddef.h>
#include <support/check.h>

/* Note: This object is not linked into the main program, so we cannot
   use delayed test failure reporting via TEST_VERIFY etc., and have
   to use FAIL_EXIT1 (or something else that calls exit).  */

void
call_dlsym (void)
{
  void *ptr = dlsym (NULL, "does not exist");
  if (ptr != NULL)
    FAIL_EXIT1 ("dlsym did not fail as expected");
}

void
call_dlopen (void)
{
  void *handle = dlopen ("tst-dlmopen-dlerror does not exist", RTLD_NOW);
  if (handle != NULL)
    FAIL_EXIT1 ("dlopen did not fail as expected");
}
