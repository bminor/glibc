/* sigaltstack wrappers.
   Copyright (C) 2019-2020 Free Software Foundation, Inc.
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

#include <support/xsignal.h>
#include <support/support.h>
#include <support/xunistd.h>
#include <support/check.h>

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/param.h> /* roundup, MAX */

#ifndef MAP_NORESERVE
# define MAP_NORESERVE 0
#endif
#ifndef MAP_STACK
# define MAP_STACK 0
#endif

/* The "cookie" returned by xalloc_sigstack points to one of these
   structures.  */
struct sigstack_desc
{
  void *alloc_base;  /* Base address of the complete allocation.  */
  size_t alloc_size; /* Size of the complete allocation.  */
  stack_t alt_stack; /* The address and size of the stack itself.  */
  stack_t old_stack; /* The previous signal stack.  */
};

void *
xalloc_sigstack (size_t size)
{
  size_t pagesize = sysconf (_SC_PAGESIZE);
  if (pagesize == -1)
    FAIL_EXIT1 ("sysconf (_SC_PAGESIZE): %m\n");

  /* Always supply at least MINSIGSTKSZ space; passing 0 as size means
     only that much space.  No matter what the number is, round it up
     to a whole number of pages.  */
  size_t stacksize = roundup (size + MINSIGSTKSZ, pagesize);

  /* The guard bands need to be large enough to intercept offset
     accesses from a stack address that might otherwise hit another
     mapping.  Make them at least twice as big as the stack itself, to
     defend against an offset by the entire size of a large
     stack-allocated array.  The minimum is 1MiB, which is arbitrarily
     chosen to be larger than any "typical" wild pointer offset.
     Again, no matter what the number is, round it up to a whole
     number of pages.  */
  size_t guardsize = roundup (MAX (2 * stacksize, 1024 * 1024), pagesize);

  struct sigstack_desc *desc = xmalloc (sizeof (struct sigstack_desc));
  desc->alloc_size = guardsize + stacksize + guardsize;
  /* Use MAP_NORESERVE so that RAM will not be wasted on the guard
     bands; touch all the pages of the actual stack before returning,
     so we know they are allocated.  */
  desc->alloc_base = xmmap (0,
                            desc->alloc_size,
                            PROT_READ|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE|MAP_STACK,
                            -1);

  xmprotect (desc->alloc_base, guardsize, PROT_NONE);
  xmprotect (desc->alloc_base + guardsize + stacksize, guardsize, PROT_NONE);
  memset (desc->alloc_base + guardsize, 0xA5, stacksize);

  desc->alt_stack.ss_sp    = desc->alloc_base + guardsize;
  desc->alt_stack.ss_flags = 0;
  desc->alt_stack.ss_size  = stacksize;

  if (sigaltstack (&desc->alt_stack, &desc->old_stack))
    FAIL_EXIT1 ("sigaltstack (new stack: sp=%p, size=%zu, flags=%u): %m\n",
                desc->alt_stack.ss_sp, desc->alt_stack.ss_size,
                desc->alt_stack.ss_flags);

  return desc;
}

void
xfree_sigstack (void *stack)
{
  struct sigstack_desc *desc = stack;

  if (sigaltstack (&desc->old_stack, 0))
    FAIL_EXIT1 ("sigaltstack (restore old stack: sp=%p, size=%zu, flags=%u): "
                "%m\n", desc->old_stack.ss_sp, desc->old_stack.ss_size,
                desc->old_stack.ss_flags);
  xmunmap (desc->alloc_base, desc->alloc_size);
  free (desc);
}

void
xget_sigstack_location (const void *stack, unsigned char **addrp, size_t *sizep)
{
  const struct sigstack_desc *desc = stack;
  *addrp = desc->alt_stack.ss_sp;
  *sizep = desc->alt_stack.ss_size;
}
