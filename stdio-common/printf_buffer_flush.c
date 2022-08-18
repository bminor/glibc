/* Flush a struct __printf_buffer.  Multibyte version.
   Copyright (C) 2022 Free Software Foundation, Inc.
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

#include <printf_buffer.h>

#include "printf_buffer-char.h"
#include "Xprintf_buffer_flush.c"

/* The __printf_buffer_flush_* functions are defined together with
   functions that are pulled in by strong references.  */
#ifndef SHARED
# pragma weak __printf_buffer_flush_to_file
#endif /* !SHARED */

static void
__printf_buffer_do_flush (struct __printf_buffer *buf)
{
  switch (buf->mode)
    {
    case __printf_buffer_mode_failed:
      return;
    case __printf_buffer_mode_to_file:
      __printf_buffer_flush_to_file ((struct __printf_buffer_to_file *) buf);
      return;
    }
  __builtin_trap ();
}
