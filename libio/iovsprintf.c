/* Copyright (C) 1993-2023 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.

   As a special exception, if you link the code in this file with
   files compiled with a GNU compiler to produce an executable,
   that does not cause the resulting executable to be covered by
   the GNU Lesser General Public License.  This exception does not
   however invalidate any other reasons why the executable file
   might be covered by the GNU Lesser General Public License.
   This exception applies to code released by its copyright holders
   in files containing the exception.  */

#include "libioP.h"

#include <printf.h>
#include <stdint.h>
#include <printf_buffer.h>

int
__vsprintf_internal (char *string, size_t maxlen,
		     const char *format, va_list args,
		     unsigned int mode_flags)
{
  struct __printf_buffer buf;

  /* When called from fortified sprintf/vsprintf, erase the destination
     buffer and try to detect overflows.  When called from regular
     sprintf/vsprintf, do not erase the destination buffer, because
     known user code relies on this behavior (even though its undefined
     by ISO C), nor try to detect overflows.  */
  if ((mode_flags & PRINTF_CHK) != 0)
    {
      string[0] = '\0';
      __printf_buffer_init (&buf, string, maxlen,
			    __printf_buffer_mode_sprintf_chk);
    }
  else
    {
      __printf_buffer_init (&buf, string, 0, __printf_buffer_mode_sprintf);
      buf.write_end = (char *) ~(uintptr_t) 0; /* End of address space.  */
    }

  __printf_buffer (&buf, format, args, mode_flags);

  /* Write the NUL terminator if there is room.  Do not use the putc
     operation to avoid overflowing the character write count.  */
  if ((mode_flags & PRINTF_CHK) != 0 && buf.write_ptr == buf.write_end)
    __chk_fail ();
  *buf.write_ptr = '\0';

  return __printf_buffer_done (&buf);
}

int
__vsprintf (char *string, const char *format, va_list args)
{
  return __vsprintf_internal (string, -1, format, args, 0);
}

ldbl_strong_alias (__vsprintf, _IO_vsprintf)
ldbl_weak_alias (__vsprintf, vsprintf)
