/* Macros for using symbol sets for running lists of functions.
Copyright (C) 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

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

#ifndef _SET_HOOKS_H

#define __need_size_t
#include <stddef.h>
#include <sys/cdefs.h>

/* Define a hook variable called NAME.  Functions put on this hook take
   arguments described by PROTO.  Use `text_set_element (NAME, FUNCTION)'
   from gnu-stabs.h to add a function to the hook.  */

#define DEFINE_HOOK(NAME, PROTO) \
const struct								      \
  {									      \
    size_t n;								      \
    void (*fn[0]) __P (PROTO);						      \
  } NAME

/* Run all the functions hooked on the set called NAME.
   Each function is called like this: `function ARGS'.  */

#define RUN_HOOK(NAME, ARGS) \
do {									      \
  size_t i;								      \
  for (i = 0; i < NAME.n; ++i)						      \
    (*NAME.fn[i]) ARGS;							      \
} while (0)

/* Define a hook variable with NAME and PROTO, and a function called RUNNER
   which calls each function on the hook in turn, with ARGS.  */

#define DEFINE_HOOK_RUNNER(name, runner, proto, args) \
DEFINE_HOOK (name, proto); void runner proto { RUN_HOOK (name, args); }


#endif
