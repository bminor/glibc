/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <posix1_lim.h>

int
main()
{
  printf("#define L_tmpnam %u\n", sizeof("/usr/tmp") + 8);
  printf("#define TMP_MAX %u\n", 52 * 52 * 52);
  puts  ("#ifdef __USE_POSIX");
  printf("#define L_ctermid %u\n", sizeof("/dev/tty"));
  printf("#define L_cuserid 9\n");
  puts  ("#endif");
  printf("#define FOPEN_MAX %u\n", OPEN_MAX);
  printf("#define FILENAME_MAX %u\n", PATH_MAX);

  exit(0);
}
