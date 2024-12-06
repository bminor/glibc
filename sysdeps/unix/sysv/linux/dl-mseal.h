/* Memory sealing.  Linux version.
   Copyright (C) 2025 Free Software Foundation, Inc.
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

#ifndef _DL_MSEAL_H
#define _DL_MSEAL_H

/* Seal the ADDR or size LEN to protect against fruthermodifications, such as
   changes on the permission flags (through mprotect), remap (through
   mmap and/or remap), shrink, destruction changes (madvise with
   MADV_DONTNEED), or change its size.  The input has the same constraints
   as the mseal syscall.

   Any error than than unsupported by the kerneltriggers a _dl_signal_error.  */
void _dl_mseal (void *addr, size_t len, const char *object) attribute_hidden;

#endif
