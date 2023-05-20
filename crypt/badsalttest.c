/* Test program for bad DES salt detection in crypt.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
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

#include <array_length.h>
#include <stddef.h>
#include <crypt.h>

#include <support/check.h>

static const char *tests[][2] =
  {
    { "no salt", "" },
    { "single char", "/" },
    { "first char bad", "!x" },
    { "second char bad", "Z%" },
    { "both chars bad", ":@" },
    { "unsupported algorithm", "$2$" },
    { "unsupported_algorithm", "_1" },
  };

static int
do_test (void)
{
  struct crypt_data cd;

  /* Mark cd as initialized before first call to crypt_r.  */
  cd.initialized = 0;

  for (size_t i = 0; i < array_length (tests); i++)
    {
      TEST_VERIFY (crypt (tests[i][0], tests[i][1]) == NULL);

      TEST_VERIFY (crypt_r (tests[i][0], tests[i][1], &cd) == NULL);
    }

  return 0;
}

#include <support/test-driver.c>
