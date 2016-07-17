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
  uint32_t path_hook:1; /* A hook was used to complete the request */
  uint32_t path:16; /* remaining bits */

  /* FREE - pointer to allocation to free.
     REALLOC - pointer to original allocation.
     POSIX_MEMALIGN - error code */
  void *ptr1;
  /* pointer to new allocation. */
  void *ptr2;
  /* requested size. */
  size_t size;
  /* FREE - internal size of deallocation.
     REALLOC - internal size of original allocation.
     MEMALIGN - alignment.
     POSIX_MEMALIGN - alignment.  */
  size_t size2;
  /* internal size of new allocation.  */
  size_t size3;
  /* Pad out to 64-bytes for future uses and mmap'd window alignment.  */
  size_t pad[2];
};

typedef struct __malloc_trace_buffer_s *__malloc_trace_buffer_ptr;

/* Initialize the trace buffer and backing file.  The file is
   overwritten if it already exists.  */
void __malloc_trace_init (char *filename);

/* All remaining functions return current count of trace records.  */

/* Pause - but don't stop - tracing.  */
size_t __malloc_trace_pause (void);

/* Resume tracing where it left off when paused.  */
size_t __malloc_trace_unpause (void);

/* Stop tracing and clean up all the trace buffer mappings.  */
size_t __malloc_trace_stop (void);

/* Sync all buffer data to file (typically a no-op on Linux).  */
size_t __malloc_trace_sync (void);


#define __MTB_TYPE_UNUSED		0

/* ptr1 is 0x1234, size is sizeof(void *) - there is one of these at
   the beginning of the trace.  */
#define __MTB_TYPE_MAGIC		255

/* ptr2 = malloc (size) */
#define __MTB_TYPE_MALLOC		1

/* ptr2 = calloc (size) */
#define __MTB_TYPE_CALLOC		2

/* free (ptr1) */
#define __MTB_TYPE_FREE			3

/* ptr2 = realloc (ptr1, size) */
#define __MTB_TYPE_REALLOC		4

/* ptr2 = memalign (size2, size) */
#define __MTB_TYPE_MEMALIGN		5

/* ptr2 = valloc (size) */
#define __MTB_TYPE_VALLOC		6

/* ptr2 = pvalloc (size) */
#define __MTB_TYPE_PVALLOC		7

/* ptr2 = posix_memalign (ptr1, size2, size)  */
#define __MTB_TYPE_POSIX_MEMALIGN	8

typedef enum {
  MSCAN_UNUSED,
  MSCAN_ARENA,
  MSCAN_HEAP,
  MSCAN_CHUNK_USED,
  MSCAN_CHUNK_FREE,
  MSCAN_FASTBIN_FREE,
  MSCAN_UNSORTED,
  MSCAN_TOP,
  MSCAN_TCACHE
} MSCAN_Types;

void __malloc_scan_chunks (void (*callback)(void * /*ptr*/, size_t /*length*/, int /*type*/));

/* Codes for the simulator/workload programs. */
#define C_NOP 0
#define C_DONE 1
#define C_MALLOC 2
#define C_CALLOC 3
#define C_REALLOC 4
#define C_FREE 5
#define C_SYNC_W 6
#define C_SYNC_R 7
#define C_ALLOC_PTRS 8
#define C_ALLOC_SYNCS 9
#define C_NTHREADS 10
#define C_START_THREAD 11
#define C_MEMALIGN 12
#define C_VALLOC 13
#define C_PVALLOC 14
#define C_POSIX_MEMALIGN 15
