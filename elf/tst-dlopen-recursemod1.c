/* Directly opened test module that gets recursively opened again.
   Copyright (C) 2024 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <support/xdlfcn.h>

int recursemod1_status;

/* Force linking against st-dlopen-recursemod2.so.  Also allows
   checking for relocation.  */
extern int recursemod2_status;
int *force_recursemod2_reference = &recursemod2_status;

static void __attribute__ ((constructor))
init (void)
{
  ++recursemod1_status;
  printf ("info: tst-dlopen-recursemod1.so constructor called (status %d)\n",
          recursemod1_status);
}

static void __attribute__ ((destructor))
fini (void)
{
  /* The recursemod1_status variable was incremented in the
     tst-dlopen-recursemod2.so constructor.  */
  printf ("info: tst-dlopen-recursemod1.so destructor called (status %d)\n",
          recursemod1_status);
  if (recursemod1_status != 2)
    {
      puts ("error: recursemod1_status == 2 expected");
      exit (1);
    }
}
