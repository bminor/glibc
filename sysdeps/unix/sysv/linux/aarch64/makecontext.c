/* Create new context.
   Copyright (C) 2002-2024 Free Software Foundation, Inc.

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

#include <sysdep.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>

#define GCS_MAGIC 0x47435300

static struct _aarch64_ctx *extension (void *p)
{
  return p;
}

struct gcs_list {
  struct gcs_list *next;
  void *base;
  size_t size;
};

static __thread struct gcs_list *gcs_list_head = NULL;

static void
record_gcs (void *base, size_t size)
{
  struct gcs_list *p = malloc (sizeof *p);
  if (p == NULL)
    abort ();
  p->base = base;
  p->size = size;
  p->next = gcs_list_head;
  gcs_list_head = p;
}

static void
free_gcs_list (void)
{
  for (;;)
    {
      struct gcs_list *p = gcs_list_head;
      if (p == NULL)
	break;
      gcs_list_head = p->next;
      __munmap (p->base, p->size);
      free (p);
    }
}

/* Called during thread shutdown to free resources.  */
void
__libc_aarch64_thread_freeres (void)
{
  free_gcs_list ();
}

#ifndef __NR_map_shadow_stack
# define __NR_map_shadow_stack 453
#endif
#ifndef SHADOW_STACK_SET_TOKEN
# define SHADOW_STACK_SET_TOKEN (1UL << 0)
# define SHADOW_STACK_SET_MARKER (1UL << 1)
#endif

static void *
map_shadow_stack (void *addr, size_t size, unsigned long flags)
{
  return (void *) INLINE_SYSCALL_CALL (map_shadow_stack, addr, size, flags);
}

#define GCS_MAX_SIZE (1UL << 31)
#define GCS_ALTSTACK_RESERVE 160

static void *
alloc_makecontext_gcs (size_t stack_size)
{
  size_t size = (stack_size / 2 + GCS_ALTSTACK_RESERVE) & -8UL;
  if (size > GCS_MAX_SIZE)
    size = GCS_MAX_SIZE;

  unsigned long flags = SHADOW_STACK_SET_MARKER | SHADOW_STACK_SET_TOKEN;
  void *base = map_shadow_stack (NULL, size, flags);
  if (base == (void *) -1)
    /* ENOSYS, bad size or OOM.  */
    abort ();

  record_gcs (base, size);

  uint64_t *gcsp = (uint64_t *) ((char *) base + size);
  /* Skip end of GCS token.  */
  gcsp--;
  /* Verify GCS cap token.  */
  gcsp--;
  if (((uint64_t)gcsp & 0xfffffffffffff000) + 1 != *gcsp)
    abort ();
  /* Return the target GCS pointer for context switch.  */
  return gcsp + 1;
}

void
__free_makecontext_gcs (void *gcs)
{
  struct gcs_list *p = gcs_list_head;
  struct gcs_list **q = &gcs_list_head;
  for (;;)
    {
      if (p == NULL)
	abort ();
      if (gcs == p->base + p->size - 8)
	break;
      q = &p->next;
      p = p->next;
    }
  *q = p->next;
  __munmap (p->base, p->size);
  free (p);
}

/* makecontext sets up a stack and the registers for the
   user context.  The stack looks like this:

               +-----------------------+
	       | padding as required   |
               +-----------------------+
    sp ->      | parameter 7-n         |
               +-----------------------+

   The registers are set up like this:
     %x0 .. %x7: parameter 1 to 8
     %x19   : uc_link
     %sp    : stack pointer.
*/

void
__makecontext (ucontext_t *ucp, void (*func) (void), int argc, ...)
{
  extern void __startcontext (void);
  uint64_t *sp;
  va_list ap;
  int i;

  sp = (uint64_t *)
    ((uintptr_t) ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size);

  /* Allocate stack arguments.  */
  sp -= argc < 8 ? 0 : argc - 8;

  /* Keep the stack aligned.  */
  sp = (uint64_t *) (((uintptr_t) sp) & -16L);

  ucp->uc_mcontext.regs[19] = (uintptr_t) ucp->uc_link;
  ucp->uc_mcontext.regs[20] = (uintptr_t) func;
  ucp->uc_mcontext.sp = (uintptr_t) sp;
  ucp->uc_mcontext.pc = (uintptr_t) __startcontext;
  ucp->uc_mcontext.regs[29] = (uintptr_t) 0;
  ucp->uc_mcontext.regs[30] = (uintptr_t) 0;

  void *p = ucp->uc_mcontext.__reserved;
  if (extension (p)->magic == FPSIMD_MAGIC)
    p = (char *)p + extension (p)->size;
  if (extension (p)->magic == GCS_MAGIC)
    {
      /* Using the kernel struct gcs_context layout.  */
      struct { uint64_t x, gcspr, y, z; } *q = p;
      q->gcspr = (uint64_t) alloc_makecontext_gcs (ucp->uc_stack.ss_size);
    }

  va_start (ap, argc);
  for (i = 0; i < argc; ++i)
    if (i < 8)
      ucp->uc_mcontext.regs[i] = va_arg (ap, uint64_t);
    else
      sp[i - 8] = va_arg (ap, uint64_t);

  va_end (ap);
}

weak_alias (__makecontext, makecontext)
