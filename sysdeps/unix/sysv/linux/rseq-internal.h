/* Restartable Sequences internal API.  Linux implementation.
   Copyright (C) 2021-2025 Free Software Foundation, Inc.

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

#ifndef RSEQ_INTERNAL_H
#define RSEQ_INTERNAL_H

#include <sysdep.h>
#include <errno.h>
#include <kernel-features.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/rseq.h>

/* Minimum size of the rseq area allocation required by the syscall.  The
   actually used rseq feature size may be less (20 bytes initially).  */
#define RSEQ_AREA_SIZE_INITIAL 32

/* Minimum used feature size of the rseq area.  */
#define RSEQ_AREA_SIZE_INITIAL_USED 20

/* Maximum currently used feature size of the rseq area.  */
#define RSEQ_AREA_SIZE_MAX_USED 28

/* Minimum alignment of the rseq area.  */
#define RSEQ_MIN_ALIGN 32

/* Alignment requirement of the rseq area.
   Populated from the auxiliary vector with a minimum of '32'.
   In .data.relro but not yet write-protected.  */
extern size_t _rseq_align attribute_hidden;

/* Size of the active features in the rseq area.
   Populated from the auxiliary vector with a minimum of '20'.
   In .data.relro but not yet write-protected.  */
extern unsigned int _rseq_size attribute_hidden;

/* Offset from the thread pointer to the rseq area.
   In .data.relro but not yet write-protected.  */
extern ptrdiff_t _rseq_offset attribute_hidden;

#ifdef RSEQ_SIG
static inline bool
rseq_register_current_thread (struct pthread *self, bool do_rseq)
{
  if (do_rseq)
    {
      unsigned int size;
#if IS_IN (rtld)
      /* Use the hidden symbol in ld.so.  */
      size = _rseq_size;
#else
      size = __rseq_size;
#endif
      if (size < RSEQ_AREA_SIZE_INITIAL)
        /* The initial implementation used only 20 bytes out of 32,
           but still expected size 32.  */
        size = RSEQ_AREA_SIZE_INITIAL;

      /* Initialize the rseq fields that are read by the kernel on
         registration, there is no guarantee that struct pthread is
         cleared on all architectures.  */
      THREAD_SETMEM (self, rseq_area.cpu_id, RSEQ_CPU_ID_UNINITIALIZED);
      THREAD_SETMEM (self, rseq_area.cpu_id_start, 0);
      THREAD_SETMEM (self, rseq_area.rseq_cs, 0);
      THREAD_SETMEM (self, rseq_area.flags, 0);

      int ret = INTERNAL_SYSCALL_CALL (rseq, &self->rseq_area,
                                       size, 0, RSEQ_SIG);
      if (!INTERNAL_SYSCALL_ERROR_P (ret))
        return true;
    }
  /* When rseq is disabled by tunables or the registration fails, inform
     userspace by setting 'cpu_id' to RSEQ_CPU_ID_REGISTRATION_FAILED.  */
  THREAD_SETMEM (self, rseq_area.cpu_id, RSEQ_CPU_ID_REGISTRATION_FAILED);
  return false;
}
#else /* RSEQ_SIG */
static inline bool
rseq_register_current_thread (struct pthread *self, bool do_rseq)
{
  THREAD_SETMEM (self, rseq_area.cpu_id, RSEQ_CPU_ID_REGISTRATION_FAILED);
  return false;
}
#endif /* RSEQ_SIG */

#endif /* rseq-internal.h */
