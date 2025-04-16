/* Undefined Behavior Sanitizer support.
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

#include <ldsodefs.h>
#include <stdarg.h>
#include <unistd.h>
#include <abort-instr.h>
#include <ubsan.h>
#include <dl-tunables.h>

static void _Noreturn
ubsan_abort (void)
{
  /* abort() pulls a lot of extra definition from libc (rwlock, signal
     hanlding, pthread, etc.; so use a more simpler implementation for
     now.  */
  raise (SIGABRT);

#ifdef ABORT_INSTRUCTION
  ABORT_INSTRUCTION;
#endif
  _exit (127);
}

void
__ubsan_error (const struct source_location *source,
	       const char *fmt,
	       ...)
{
  _dl_debug_printf_c ("UBSAN: Undefined behaviour in %s:%u:%u ",
		      get_source_location_file_name (source),
		      get_source_location_line (source),
		      get_source_location_column (source));

  va_list ap;
  va_start (ap, fmt);
  _dl_debug_vprintf_c (fmt, ap);
  va_end (ap);

  if (TUNABLE_GET (glibc, ubsan, halt_on_errors, int32_t, NULL))
    ubsan_abort ();
}
