/* Determine the "endianess" of the CPU.
   Copyright (C) 1991 Free Software Foundation, Inc.
   Contributed by Torbjorn Granlund (tege@sics.se).

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

#include <stdio.h>

main()
{
  unsigned long int i;

  puts("#ifndef\t_ENDIAN_H\n#define\t_ENDIAN_H\t1");

  if (sizeof(i) != 4)
    puts("#error \"Not a 32-bit machine!\"");

  i = (((((('A' << 8) + 'B') << 8) + 'C') << 8) + 'D');

  if (!strncmp ("ABCD", (char *) &i, 4))
    puts ("#define __BIG_ENDIAN");
  else if (!strncmp ("DCBA", (char *) &i, 4))
    puts ("#define __LITTLE_ENDIAN");
  else
    puts ("#define __MIXED_ENDIAN");

  puts("#endif\t/* endian.h */");

  exit (0);
}
