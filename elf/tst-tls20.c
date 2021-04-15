/* Test dtv setup if entries don't have monotone increasing generation.
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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xdlfcn.h>
#include <support/xthread.h>

#define NMOD 100
static void *mod[NMOD];

static void
load_fail (void)
{
  /* Expected to fail because of a missing symbol.  */
  void *m = dlopen ("tst-tls20mod-bad.so", RTLD_NOW);
  if (m != NULL)
    FAIL_EXIT1 ("dlopen of tst-tls20mod-bad.so succeeded\n");
}

static void
load_mod (int i)
{
  char *buf = xasprintf ("tst-tls-manydynamic%02dmod.so", i);
  mod[i] = xdlopen (buf, RTLD_LAZY);
  free (buf);
}

static void
unload_mod (int i)
{
  if (mod[i] != NULL)
    xdlclose (mod[i]);
  mod[i] = NULL;
}

static void
access (int i)
{
  char *buf = xasprintf ("tls_global_%02d", i);
  dlerror ();
  int *p = dlsym (mod[i], buf);
  printf ("mod[%d]: &tls = %p\n", i, p);
  if (p == NULL)
    FAIL_EXIT1 ("dlsym failed: %s\n", dlerror ());
  ++*p;
  free (buf);
}

static void *
start (void *a)
{
  for (int i = 0; i < NMOD; i++)
    if (mod[i] != NULL)
      access (i);
  return 0;
}

static int
do_test (void)
{
  int i;

  for (i = 0; i < NMOD; i++)
    {
      load_mod (i);
      /* Bump the generation of mod[0] without using new dtv slot.  */
      unload_mod (0);
      load_fail (); /* Ensure GL(dl_tls_dtv_gaps) is true: see bug 27135.  */
      load_mod (0);
      /* Access TLS in all loaded modules.  */
      pthread_t t = xpthread_create (0, start, 0);
      xpthread_join (t);
    }
  for (i = 0; i < NMOD; i++)
    unload_mod (i);
  return 0;
}

#include <support/test-driver.c>
