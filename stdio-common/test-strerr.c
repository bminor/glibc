/* Test strerrorname_np and strerrordesc_np.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#include <string.h>
#include <errno.h>
#include <array_length.h>

#include <support/support.h>
#include <support/check.h>

#define N_(name)      name

static const char *const errlist[] =
  {
/* This file is auto-generated from errlist.def.  */
#include <errlist.h>
  };

#define MSGSTR_T errname_t
#define MSGSTR   errname
#define MSGIDX   errnameidx
#include <errlist-name.h>
#undef MSGSTR
#undef MSGIDX

static int
do_test (void)
{
  TEST_VERIFY (strerrordesc_np (-1) == NULL);
  TEST_VERIFY (strerrordesc_np (array_length (errlist)) == NULL);
  for (size_t i = 0; i < array_length (errlist); i++)
    {
      if (errlist[i] == NULL)
        continue;
      TEST_COMPARE_STRING (strerrordesc_np (i), errlist[i]);
    }

  TEST_VERIFY (strerrorname_np (-1) == NULL);
  TEST_VERIFY (strerrorname_np (array_length (errlist)) == NULL);
  for (size_t i = 0; i < array_length (errlist); i++)
    {
      if (errlist[i] == NULL)
        continue;
      TEST_COMPARE_STRING (strerrorname_np (i), errname.str + errnameidx[i]);
    }

  return 0;
}

#include <support/test-driver.c>
