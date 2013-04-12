/* Entry-point for programs.  NaCl version.
   Copyright (C) 2013 Free Software Foundation, Inc.
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

#include <stdint.h>
#include <link.h>

/* NaCl's elf32.h is incompatible with the real <elf.h>.  */
#define NATIVE_CLIENT_SRC_INCLUDE_ELF32_H_
#include <native_client/src/untrusted/nacl/nacl_startup.h>


/* The application defines this, of course.  */
extern int main (int argc, char **argv, char **envp);

/* These are defined in libc.  */
extern int __libc_csu_init (int argc, char **argv, char **envp);
extern void __libc_csu_fini (void);
extern void __libc_start_main (int (*main) (int, char **, char **),
			       int argc, char **argv, ElfW(auxv_t) *auxv,
			       int (*init) (int, char **, char **),
			       void (*fini) (void),
			       void (*rtld_fini) (void),
			       void *stack_end);

void
_start (uint32_t info[])
{
  /* The generic code actually assumes that envp follows argv.  */

  __libc_start_main (&main,
		     nacl_startup_argc (info),
		     nacl_startup_argv (info),
		     nacl_startup_auxv (info),
		     &__libc_csu_init, &__libc_csu_fini,
		     nacl_startup_fini (info),
		     __builtin_frame_address (0));

  /* That should not return.  Make sure we crash if it did.  */
  while (1)
    *(volatile int *) 0;
}
