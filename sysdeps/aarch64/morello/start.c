/* Copyright (C) 2022 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <ldsodefs.h>
#include <cheri-rel.h>

/* This is the canonical entry point, usually the first thing in the text
   segment.

   Note that in case of dynamic linked exe the code in the .init section
   has already been run.  This includes _init and _libc_init.

   At this entry point, most registers' values are unspecified, except:

   x0		argc

   c1		argv

   c2		envp

   c3		auxv

   c4		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs. It is unspecified in a static linked
		executable.

   csp		The stack pointer.
 */
asm(""
".global	_start\n"
".type		_start, %function\n"
"_start:\n"
"	.cfi_startproc\n"
"	.cfi_undefined c30\n"
"	mov	c29, czr\n"
"	mov	c30, czr\n"
"	mov	c5, csp\n"
"	b	__real_start\n"
"	.cfi_endproc\n"
"	.size _start, .-_start\n");

#ifndef SHARED
static int
is_static_linked (void)
{
  unsigned long x;
  asm (""
    ".weak __rela_dyn_start\n"
    ".hidden __rela_dyn_start\n"
    "movz %0, #:abs_g3:__rela_dyn_start\n"
    "movk %0, #:abs_g2_nc:__rela_dyn_start\n"
    "movk %0, #:abs_g1_nc:__rela_dyn_start\n"
    "movk %0, #:abs_g0_nc:__rela_dyn_start\n" : "=r"(x));
  return x != 0;
}

static uintptr_t
get_rela_dyn_start (void)
{
  uintptr_t p;
  asm (""
    ".weak __rela_dyn_start\n"
    ".hidden __rela_dyn_start\n"
    "adrp %0, __rela_dyn_start\n"
    "add %0, %0, :lo12:__rela_dyn_start\n" : "=r"(p));
  return p;
}

static uintptr_t
get_rela_dyn_end (void)
{
  uintptr_t p;
  asm (""
    ".weak __rela_dyn_end\n"
    ".hidden __rela_dyn_end\n"
    "adrp %0, __rela_dyn_end\n"
    "add %0, %0, :lo12:__rela_dyn_end\n" : "=r"(p));
  return p;
}

static void
get_caps (uintptr_t *cap_rx, uintptr_t *cap_rw, const uintptr_t *auxv)
{
  for (;;)
    {
      switch ((unsigned long)auxv[0])
	{
	case AT_NULL:
	  return;
	case AT_CHERI_EXEC_RX_CAP:
	  *cap_rx = auxv[1];
	  *cap_rx = __builtin_cheri_perms_and (*cap_rx, CAP_PERM_MASK_RX);
	  break;
	case AT_CHERI_EXEC_RW_CAP:
	  *cap_rw = auxv[1];
	  *cap_rw = __builtin_cheri_perms_and (*cap_rw, CAP_PERM_MASK_RW);
	  break;
	}
      auxv += 2;
    }
}

static void
apply_rel (uintptr_t cap_rx, uintptr_t cap_rw, uintptr_t start, uintptr_t end)
{
  const ElfW(Rela) *r;
  for (r = (const ElfW(Rela) *)start; r != (void *)end; r++)
    {
      uintptr_t *reloc_addr =
	(uintptr_t *) __builtin_cheri_address_set (cap_rw, r->r_offset);
      uintptr_t value = morello_relative (0, cap_rx, cap_rw, r, reloc_addr);
      *reloc_addr = value;
    }
}
#endif /* !SHARED */

int main (int argc, char **argv, char **envp, void *auxv);

void __libc_start_main (int main (int, char **, char **, void *),
			int argc, char **argv, char **envp, void *auxv,
			void rtld_fini (void), void *sp);

void
__real_start (int argc, char **argv, char **envp, void *auxv,
	      void (*rtld_fini) (void), uintptr_t *sp)
{
#ifndef SHARED
  if (is_static_linked ())
    {
      uintptr_t start = get_rela_dyn_start ();
      uintptr_t end = get_rela_dyn_end ();
      uintptr_t cap_rx = 0;
      uintptr_t cap_rw = 0;
      get_caps (&cap_rx, &cap_rw, auxv);
      apply_rel (cap_rx, cap_rw, start, end);
      rtld_fini = 0;
    }
  /* Compiler barrier after relocs are processed.  */
  asm volatile ("" ::: "memory");
#endif
  __libc_start_main (main, argc, argv, envp, auxv, rtld_fini, sp);
  __builtin_trap ();
}

int __data_start = 1;
weak_alias (__data_start, data_start);
