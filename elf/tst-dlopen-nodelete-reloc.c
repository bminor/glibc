/* Test propagation of NODELETE to an already-loaded object via relocation.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

/* This test exercises NODELETE propagation due to data relocations
   and unique symbols.

   First test: Global scope variant, data relocation as the NODELETE
   trigger.  mod1 is loaded first.

      mod2 ---(may_finalize_mod1 relocation dependency)---> mod1
    (NODELETE)                                   (marked as NODELETE)

   Second test: Local scope variant, data relocation.  mod3 is loaded
   first, then mod5.

      mod5 ---(DT_NEEDED)--->  mod4  ---(DT_NEEDED)---> mod3
    (NODELETE)           (not NODELETE)                  ^
        \                                               / (marked as
         `--(may_finalize_mod3 relocation dependency)--/   NODELETE)

   Third test: Shared local scope with unique symbol.  mod6 is loaded
   first, then mod7.  No explicit dependencies between the two objects.

      mod7 ---(unique symbol)---> mod6
                          (marked as NODELETE)

   Forth test: Non-shared scopes with unique symbol.  mod8 and mod10
   are loaded from the main program.  mod8 loads mod9 from an ELF
   constructor, mod10 loads mod11.  There are no DT_NEEDED
   dependencies.  The unique symbol dependency is:

      mod9 ---(unique symbol)---> mod11
                          (marked as NODELETE)

   Fifth test: Shared local scope with unique symbol, like test 3, but
   this time, there is also a DT_NEEDED dependency:

                 DT_NEEDED
      mod13 ---(unique symbol)---> mod12
                          (marked as NODELETE)
   */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <support/check.h>
#include <support/xdlfcn.h>

static int
do_test (void)
{
#if 0
  /* First case: global scope, regular data symbol.  Open the object
     which is not NODELETE initially.  */
  void *mod1 = xdlopen ("tst-dlopen-nodelete-reloc-mod1.so",
                        RTLD_NOW | RTLD_GLOBAL);
  /* This is used to indicate that the ELF destructor may be
     called.  */
  bool *may_finalize_mod1 = xdlsym (mod1, "may_finalize_mod1");
  /* Open the NODELETE object.  */
  void *mod2 = xdlopen ("tst-dlopen-nodelete-reloc-mod2.so", RTLD_NOW);
  /* This has no effect because the DSO is directly marked as
     NODELETE.  */
  xdlclose (mod2);
  /* This has no effect because the DSO has been indirectly marked as
     NODELETE due to a relocation dependency.  */
  xdlclose (mod1);
#endif

  /* Second case: local scope, regular data symbol.  Open the object
     which is not NODELETE initially.  */
  void *mod3 = xdlopen ("tst-dlopen-nodelete-reloc-mod3.so", RTLD_NOW);
  bool *may_finalize_mod3 = xdlsym (mod3, "may_finalize_mod3");
  /* Open the NODELETE object.  */
  void *mod5 = xdlopen ("tst-dlopen-nodelete-reloc-mod5.so", RTLD_NOW);
  /* Again those have no effect because of NODELETE.  */
  //xdlclose (mod5);
  (void) mod5;
  xdlclose (mod3);

  /* Third case: Unique symbol.  */
  void *mod6 = xdlopen ("tst-dlopen-nodelete-reloc-mod6.so", RTLD_NOW);
  bool *may_finalize_mod6 = xdlsym (mod6, "may_finalize_mod6");
  void *mod7 = xdlopen ("tst-dlopen-nodelete-reloc-mod7.so", RTLD_NOW);
  bool *may_finalize_mod7 = xdlsym (mod7, "may_finalize_mod7");
  /* This should not have any effect because of the unique symbol. */
  xdlclose (mod6);
  /* mod7 is not NODELETE and can be closed.  */
  *may_finalize_mod7 = true;
  xdlclose (mod7);

  /* Fourth case: Unique symbol, indirect loading.  */
  void *mod8 = xdlopen ("tst-dlopen-nodelete-reloc-mod8.so", RTLD_NOW);
  void *mod9 = xdlopen ("tst-dlopen-nodelete-reloc-mod9.so",
                        RTLD_NOW | RTLD_NOLOAD);
  bool *may_finalize_mod9 = xdlsym (mod9, "may_finalize_mod9");
  xdlclose (mod9);              /* Drop mod9 reference.  */
  void *mod10 = xdlopen ("tst-dlopen-nodelete-reloc-mod10.so", RTLD_NOW);
  void *mod11 = xdlopen ("tst-dlopen-nodelete-reloc-mod11.so",
                        RTLD_NOW | RTLD_NOLOAD);
  bool *may_finalize_mod11 = xdlsym (mod11, "may_finalize_mod11");
  xdlclose (mod11);              /* Drop mod11 reference.  */
  /* This should not have any effect because of the unique symbol. */
  xdlclose (mod6);
  /* mod11 is not NODELETE and can be closed.  */
  *may_finalize_mod11 = true;
  /* Trigger closing of mod11, too.  */
  xdlclose (mod10);
  /* Does not trigger closing of mod9.  */
  *may_finalize_mod9 = true;
  xdlclose (mod8);

  /* Fifth case: Unique symbol, with DT_NEEDED dependency.  */
  void *mod12 = xdlopen ("tst-dlopen-nodelete-reloc-mod12.so", RTLD_NOW);
  bool *may_finalize_mod12 = xdlsym (mod12, "may_finalize_mod12");
  void *mod13 = xdlopen ("tst-dlopen-nodelete-reloc-mod13.so", RTLD_NOW);
  bool *may_finalize_mod13 = xdlsym (mod13, "may_finalize_mod13");
  /* This should not have any effect because of the unique symbol. */
  xdlclose (mod12);
  /* mod13 is not NODELETE and can be closed.  */
  *may_finalize_mod13 = true;
  xdlclose (mod13);

  /* Prepare for the process exit.  */
//  *may_finalize_mod1 = true;
  *may_finalize_mod3 = true;
  *may_finalize_mod6 = true;
  //*may_finalize_mod9 = true;
  *may_finalize_mod12 = true;
  return 0;
}

#include <support/test-driver.c>
