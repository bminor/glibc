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
"	mov	c0, csp\n"
"	bl	__real_start\n"
	/* Jump to the user's entry point, with original csp.  */
"	mov     c16, c0\n"
"	mov     c0, c1\n"
"	br      c16\n"
"	.cfi_endproc\n"
"	.size _start, .-_start\n");

typedef void (entry_t) (void (*)(void));

typedef struct user_entry {
	entry_t *fun;
	void (*arg)(void);
};

struct user_entry
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
  struct user_entry e = {entry, _dl_fini};
  return e;
}

/* Generic ld.so start code in rtld.c.  */
uintptr_t
_dl_start (void *) attribute_hidden;

/* ld.so entry point.  */
struct user_entry
__real_start (uintptr_t *sp)
{
  /* Run ls.so setup.  */
  entry_t *entry = (entry_t *) _dl_start (sp);
  return _dl_start_user (sp, entry);
}
