/* x86 version of hardware capability information handling macros.
   Copyright (C) 2017 Free Software Foundation, Inc.

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
   <http://www.gnu.org/licenses/>.  */

#define _DL_HWCAP_COUNT 2

enum
{
  HWCAP_X86_SSE2  = 1 << 0,
  HWCAP_X86_AVX2  = 1 << 1
};

static inline const char *
__attribute__ ((unused))
_dl_hwcap_string (int idx)
{
  return GLRO(dl_x86_hwcap_flags)[idx];
};

static inline int
__attribute__ ((unused, always_inline))
_dl_string_hwcap (const char *str)
{
  int i;

  for (i = 0; i < _DL_HWCAP_COUNT; i++)
    {
      if (strcmp (str, GLRO(dl_x86_hwcap_flags)[i]) == 0)
	return i;
    }
  return -1;
};

/* We cannot provide a general printing function.  */
#define _dl_procinfo(type, word) -1
