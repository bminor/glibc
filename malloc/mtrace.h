/* Malloc trace buffer API and definitions.
   Copyright (C) 2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by DJ Delorie <dj@redhat.com>.

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
   not, see <http://www.gnu.org/licenses/>.  */

/* The trace buffer contains a minimal amount of information which can
   be used to help reconstruct the memory profile of the calling
   application.  */

struct __malloc_trace_buffer_s {
  int32_t thread; /* if thread_t > 32 bits, this is the LSBs */

  uint32_t type:8; /* call type - malloc, free, etc */

  uint32_t path_thread_cache:1; /* malloc/free interacted with the thread cache */
  uint32_t path_cpu_cache:1; /* malloc/free interacted with the per-cpu cache */
  uint32_t path_cpu_cache2:1; /* per-cpu cache contention required realloc */
  uint32_t path_sbrk:1; /* sbrk was called */
  uint32_t path_mmap:1; /* mmap was called */
  uint32_t path_munmap:1; /* munmap was called */
  uint32_t path_m_f_realloc:1; /* realloc became malloc/free (i.e. next few records) */
  uint32_t path:17; /* remaining bits */

  uint64_t ptr1;
  uint64_t ptr2;
  uint64_t size;
};

typedef struct __malloc_trace_buffer_s *__malloc_trace_buffer_ptr;

/* These three are only valid inside glibc proper.  */
extern volatile __malloc_trace_buffer_ptr __malloc_trace_buffer;
extern volatile int __malloc_trace_buffer_size;
extern volatile int __malloc_trace_buffer_head;

/* bufptr is a random chunk of memory, bufsize is the size of that
   chunk in BYTES.  Returns the size of __malloc_trace_buffer_s.  The
   buffer should be filled with NUL bytes before passing, such that
   each record's type is UNUSED (below).  The trace buffer may be
   disabled by passing NULL,0 although it's up to the caller to obtain
   and free/unmap the previous buffer first.  */
int __malloc_set_trace_buffer (void *bufptr, int bufsize);

/* Returns the location of the buffer (same as passed above, or NULL).
   Also fills in BUFCOUNT which is the number of records (not bytes)
   in the buffer, and BUFHEAD which is the index of the most recently
   filled entry.  NOTE that BUFHEAD might be greater than bufcount; if
   so it reflects the number of records that would have been stored
   had there been size, and the caller must modulo that by BUFCOUNT to
   get the ending index.  The last BUFCOUNT records are stored;
   earlier records are overwritten. */
void * __malloc_get_trace_buffer (int *bufcount, int *bufhead);


#define __MTB_TYPE_UNUSED	0

/* ptr2 = malloc (size) */
#define __MTB_TYPE_MALLOC	1

/* ptr2 = calloc (size) */
#define __MTB_TYPE_CALLOC	2

/* free (ptr1) */
#define __MTB_TYPE_FREE		3

/* ptr2 = realloc (ptr1, size) */
#define __MTB_TYPE_REALLOC	4

/* ptr2 = memalign (size, (int)ptr2) */
#define __MTB_TYPE_MEMALIGN	5

/* ptr2 = valloc (size) */
#define __MTB_TYPE_VALLOC	6
