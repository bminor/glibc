/* Print CPU diagnostics data in ld.so.  PowerPC version.
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

#include <dl-diagnostics.h>
#include <dl-hwcap-info.h>
#include <ldsodefs.h>
#include <sys/auxv.h>

static void
print_hwcap_value (const char *label, int hwcap, unsigned long int value)
{
  _dl_printf ("powerpc.cpu_features.%s=\"", label);
  size_t offset = 0;
  for (int i = 0; i < __dl_hwcap_info_size; ++i)
    {
      const char *hwcap_name = __dl_hwcap_names + offset;
      size_t hwcap_name_len = strlen (hwcap_name);
      if (hwcap == __dl_hwcap_info[i].hwcap
	  && value & __dl_hwcap_info[i].value)
	_dl_printf ("%s ", hwcap_name);
      offset += hwcap_name_len + 1;
    }
  _dl_printf ("\"\n");
}

void
_dl_diagnostics_cpu (void)
{
  print_hwcap_value ("hwcap", AT_HWCAP, GLRO(dl_hwcap));
  print_hwcap_value ("hwcap2", AT_HWCAP2, GLRO(dl_hwcap2));
  print_hwcap_value ("hwcap3", AT_HWCAP3, GLRO(dl_hwcap3));
  print_hwcap_value ("hwcap4", AT_HWCAP4, GLRO(dl_hwcap4));
}
