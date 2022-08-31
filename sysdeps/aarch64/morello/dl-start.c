/* ld.so _start code.
   Copyright (C) 2022 Free Software Foundation, Inc.

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

#include <assert.h>
#include <ldsodefs.h>

asm(""
".global	_start\n"
".type		_start, %function\n"
"_start:\n"
"	.cfi_startproc\n"
"	.cfi_undefined c30\n"
"	mov	c29, czr\n"
"	mov	c30, czr\n"
"	b	__real_start\n"
"	.cfi_endproc\n"
"	.size _start, .-_start\n");

typedef void (entry_t) (int, char **, char **, void *, void (*)(void));

__attribute__ ((noinline, noreturn))
void
_dl_start_user (uintptr_t *args, entry_t *entry)
{
  /* Setup argv, envp, auxv for the application.  */
  uintptr_t *p;
  long n;
  int argc = args[0];
  p = args + 1;
  n = argc + 1;
  char **argv = (char **) __builtin_cheri_bounds_set (p, n * sizeof *p);
  p += n;
  for (n = 0; p[n]; n++);
  n++;
  char **envp = (char **) __builtin_cheri_bounds_set (p, n * sizeof *p);
  p += n;
  for (n = 0; p[n] != AT_NULL; n += 2);
  n += 2;
  uintptr_t *auxv = __builtin_cheri_bounds_set (p, n * sizeof *p);

  _dl_init (GL(dl_ns)[LM_ID_BASE]._ns_loaded, argc, argv, envp);
  entry (argc, argv, envp, auxv, _dl_fini);
  __builtin_trap ();
}

/* Count the array length needed for traditional ELF entry.  */
static inline long
count_args (int argc, char **argv, char **envp, uintptr_t *auxv)
{
  char **p;
  uintptr_t *q;
  long nargs = argc + 2;
  for (p = envp; *p != NULL; p++);
  nargs += p - envp + 1;
  for (q = auxv; *q != AT_NULL; q += 2);
  nargs += q - auxv + 2;
  return nargs;
}

/* Generic ld.so start code in rtld.c.  */
uintptr_t
_dl_start (void *) attribute_hidden;

/* ld.so entry point.  */
void
__real_start (int argc, char **argv, char **envp, void *auxv)
{
  long nargs = count_args (argc, argv, envp, auxv);
  {
    /* _dl_start requires continuous argv, envp, auxv.  */
    uintptr_t args[nargs];
    long i = 0, j;
    args[i++] = argc;
    for (j = 0; argv[j] != NULL; j++)
      args[i++] = (uintptr_t) argv[j];
    args[i++] = 0;
    for (j = 0; envp[j] != NULL; j++)
      args[i++] = (uintptr_t) envp[j];
    args[i++] = 0;
    uintptr_t *a = auxv;
    for (j = 0; a[j] != AT_NULL; j += 2)
      {
	args[i++] = a[j];
	args[i++] = a[j+1];
      }
    args[i++] = AT_NULL;
    args[i++] = 0;
    assert (i == nargs);

    /* Run ls.so setup.  */
    entry_t *entry = (entry_t *) _dl_start (args);
    _dl_start_user (args, entry);
  }
}
