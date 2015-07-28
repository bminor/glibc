/* Copyright (C) 2015 Free Software Foundation, Inc.
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

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthreaddef.h>

__thread int tlsvar;

__thread int iexecvar __attribute__ ((tls_model ("initial-exec")));

void *
toucher (void *arg)
{
  tlsvar++;

  printf ("tlsvar=%d iexecvar=%d\n", tlsvar, iexecvar);

  iexecvar++;

  printf ("tlsvar=%d iexecvar=%d\n", tlsvar, iexecvar);

  return NULL;
}
