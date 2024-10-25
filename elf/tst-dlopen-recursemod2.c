/* Indirectly opened module that recursively opens the directly opened module.
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

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int recursemod2_status;

static void __attribute__ ((constructor))
init (void)
{
  ++recursemod2_status;
  printf ("info: tst-dlopen-recursemod2.so constructor called (status %d)\n",
          recursemod2_status);
  void *handle = dlopen ("tst-dlopen-recursemod1.so", RTLD_NOW);
  if (handle == NULL)
    {
      printf ("error: dlopen: %s\n", dlerror ());
      exit (1);
    }
  int *status = dlsym (handle, "recursemod1_status");
  if (status == NULL)
    {
      printf ("error: dlsym: %s\n", dlerror ());
      exit (1);
    }
  printf ("info: recursemod1_status == %d\n", *status);
  if (*status != 1)
    {
      puts ("error: recursemod1_status == 1 expected");
      exit (1);
    }
  ++*status;
  printf ("info: recursemod1_status == %d\n", *status);

  int **mod2_status = dlsym (handle, "force_recursemod2_reference");
  if (mod2_status == NULL || *mod2_status != &recursemod2_status)
    {
      puts ("error: invalid recursemod2_status address in"
            " tst-dlopen-recursemod1.so");
      exit (1);
    }
}

static void __attribute__ ((destructor))
fini (void)
{
  printf  ("info: tst-dlopen-recursemod2.so destructor called (status %d)\n",
           recursemod2_status);
}
