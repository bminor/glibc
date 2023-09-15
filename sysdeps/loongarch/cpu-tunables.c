/* LoongArch CPU feature tuning.
   This file is part of the GNU C Library.
   Copyright (C) 2023 Free Software Foundation, Inc.

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

# include <stdbool.h>
# include <stdint.h>
# include <unistd.h>		/* Get STDOUT_FILENO for _dl_printf.  */
# include <elf/dl-tunables.h>
# include <string.h>
# include <cpu-features.h>
# include <ldsodefs.h>
# include <sys/auxv.h>

# define HWCAP_LOONGARCH_IFUNC \
  (HWCAP_LOONGARCH_UAL | HWCAP_LOONGARCH_LSX | HWCAP_LOONGARCH_LASX)

# define CHECK_GLIBC_IFUNC_CPU_OFF(f, name, len)			\
  _Static_assert (sizeof (#name) - 1 == len, #name " != " #len);	\
  if (!memcmp (f, #name, len) &&					\
      (GLRO (dl_hwcap) & HWCAP_LOONGARCH_##name))			\
    {									\
      hwcap |= (HWCAP_LOONGARCH_##name | (~HWCAP_LOONGARCH_IFUNC));	\
      break;								\
    }									\

attribute_hidden
void
TUNABLE_CALLBACK (set_hwcaps) (tunable_val_t *valp)
{
  const char *p = valp->strval;
  size_t len;
  unsigned long hwcap = 0;
  const char *c;

  do {
      for (c = p; *c != ','; c++)
	if (*c == '\0')
	  break;

      len = c - p;

      switch(len)
      {
	default:
	  _dl_fatal_printf (
	    "The valid values of glibc.cpu.hwcaps is UAL, LASX, LSX!!\n"
			    );
	  break;
	case 3:
	  {
	    CHECK_GLIBC_IFUNC_CPU_OFF (p, LSX, 3);
	    CHECK_GLIBC_IFUNC_CPU_OFF (p, UAL, 3);
	    _dl_fatal_printf (
		"Some features are invalid or not supported on this machine!!\n"
		"The valid values of glibc.cpu.hwcaps is UAL, LASX, LSX!!\n"
                       );
	  }
	  break;
	case 4:
	  {
	    CHECK_GLIBC_IFUNC_CPU_OFF (p, LASX, 4);
	    _dl_fatal_printf (
		"Some features are invalid or not supported on this machine!!\n"
		"The valid values of glibc.cpu.hwcaps is UAL, LASX, LSX!!\n"
                       );
	  }
	  break;
      }

      p += len + 1;
    }
  while (*c != '\0');

  GLRO (dl_larch_cpu_features).hwcap &= hwcap;
}
