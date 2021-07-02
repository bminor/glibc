/* Malloc implementation for multiple threads without lock contention.
   Copyright (C) 2001-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Wolfram Gloger <wg@malloc.de>, 2001.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

/* What to do if the standard debugging hooks are in place and a
   corrupt pointer is detected: do nothing (0), print an error message
   (1), or call abort() (2). */

/* Hooks for debugging versions.  The initial hooks just call the
   initialization routine, then do the normal work. */

static void *
malloc_hook_ini (size_t sz, const void *caller)
{
  __malloc_hook = NULL;
  ptmalloc_init ();
  return __libc_malloc (sz);
}

static void *
realloc_hook_ini (void *ptr, size_t sz, const void *caller)
{
  __malloc_hook = NULL;
  __realloc_hook = NULL;
  ptmalloc_init ();
  return __libc_realloc (ptr, sz);
}

static void *
memalign_hook_ini (size_t alignment, size_t sz, const void *caller)
{
  __memalign_hook = NULL;
  ptmalloc_init ();
  return __libc_memalign (alignment, sz);
}

#include "malloc-check.c"

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_25)

/* Support for restoring dumped heaps contained in historic Emacs
   executables.  The heap saving feature (malloc_get_state) is no
   longer implemented in this version of glibc, but we have a heap
   rewriter in malloc_set_state which transforms the heap into a
   version compatible with current malloc.  */

#define MALLOC_STATE_MAGIC   0x444c4541l
#define MALLOC_STATE_VERSION (0 * 0x100l + 5l) /* major*0x100 + minor */

struct malloc_save_state
{
  long magic;
  long version;
  mbinptr av[NBINS * 2 + 2];
  char *sbrk_base;
  int sbrked_mem_bytes;
  unsigned long trim_threshold;
  unsigned long top_pad;
  unsigned int n_mmaps_max;
  unsigned long mmap_threshold;
  int check_action;
  unsigned long max_sbrked_mem;
  unsigned long max_total_mem;	/* Always 0, for backwards compatibility.  */
  unsigned int n_mmaps;
  unsigned int max_n_mmaps;
  unsigned long mmapped_mem;
  unsigned long max_mmapped_mem;
  int using_malloc_checking;
  unsigned long max_fast;
  unsigned long arena_test;
  unsigned long arena_max;
  unsigned long narenas;
};

/* Dummy implementation which always fails.  We need to provide this
   symbol so that existing Emacs binaries continue to work with
   BIND_NOW.  */
void *
attribute_compat_text_section
malloc_get_state (void)
{
  __set_errno (ENOSYS);
  return NULL;
}
compat_symbol (libc, malloc_get_state, malloc_get_state, GLIBC_2_0);

int
attribute_compat_text_section
malloc_set_state (void *msptr)
{
  struct malloc_save_state *ms = (struct malloc_save_state *) msptr;

  if (ms->magic != MALLOC_STATE_MAGIC)
    return -1;

  /* Must fail if the major version is too high. */
  if ((ms->version & ~0xffl) > (MALLOC_STATE_VERSION & ~0xffl))
    return -2;

  /* We do not need to perform locking here because malloc_set_state
     must be called before the first call into the malloc subsytem
     (usually via __malloc_initialize_hook).  pthread_create always
     calls calloc and thus must be called only afterwards, so there
     cannot be more than one thread when we reach this point.  */

  /* Disable the malloc hooks (and malloc checking).  */
  __malloc_hook = NULL;
  __realloc_hook = NULL;
  __free_hook = NULL;
  __memalign_hook = NULL;
  using_malloc_checking = 0;

  /* Patch the dumped heap.  We no longer try to integrate into the
     existing heap.  Instead, we mark the existing chunks as mmapped.
     Together with the update to dumped_main_arena_start and
     dumped_main_arena_end, realloc and free will recognize these
     chunks as dumped fake mmapped chunks and never free them.  */

  /* Find the chunk with the lowest address with the heap.  */
  mchunkptr chunk = NULL;
  {
    size_t *candidate = (size_t *) ms->sbrk_base;
    size_t *end = (size_t *) (ms->sbrk_base + ms->sbrked_mem_bytes);
    while (candidate < end)
      if (*candidate != 0)
	{
	  chunk = mem2chunk ((void *) (candidate + 1));
	  break;
	}
      else
	++candidate;
  }
  if (chunk == NULL)
    return 0;

  /* Iterate over the dumped heap and patch the chunks so that they
     are treated as fake mmapped chunks.  */
  mchunkptr top = ms->av[2];
  while (chunk < top)
    {
      if (inuse (chunk))
	{
	  /* Mark chunk as mmapped, to trigger the fallback path.  */
	  size_t size = chunksize (chunk);
	  set_head (chunk, size | IS_MMAPPED);
	}
      chunk = next_chunk (chunk);
    }

  /* The dumped fake mmapped chunks all lie in this address range.  */
  dumped_main_arena_start = (mchunkptr) ms->sbrk_base;
  dumped_main_arena_end = top;

  return 0;
}
compat_symbol (libc, malloc_set_state, malloc_set_state, GLIBC_2_0);

#endif	/* SHLIB_COMPAT */

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
