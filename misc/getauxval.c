/* Copyright (C) 2012-2022 Free Software Foundation, Inc.
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

#include <sys/auxv.h>
#include <errno.h>
#include <ldsodefs.h>
#include <stdbool.h>

void *
__getauxptr (unsigned long int type)
{
  /* error if asking for a non-pointer from getauxptr(). This list is not a
     perfect enforcement as it currently supports both transitional and draft
     ABIs, which have different capability entries.  */
  switch (type) {
    case AT_ENTRY:
    case AT_PHDR:
    case AT_BASE:
    case AT_SYSINFO_EHDR:
    case AT_EXECFN:
    case AT_RANDOM:
    case AT_PLATFORM:
    case AT_CHERI_EXEC_RW_CAP:
    case AT_CHERI_EXEC_RX_CAP:
    case AT_CHERI_INTERP_RW_CAP:
    case AT_CHERI_INTERP_RX_CAP:
    case AT_CHERI_SEAL_CAP:
    {
      ElfW(auxv_t) *p;
      for (p = GLRO(dl_auxv); p->a_type != AT_NULL; p++)
        if (p->a_type == type)
          return (void *) p->a_un.a_val;
    }
  }

  __set_errno (ENOENT);
  return 0;
}
weak_alias (__getauxptr, getauxptr)
libc_hidden_def (__getauxptr)

bool
__getauxval2 (unsigned long int type, unsigned long int *result)
{
#ifdef HAVE_AUX_VECTOR
  ElfW(auxv_t) *p;
#endif

  if (type == AT_HWCAP)
    {
      *result = GLRO(dl_hwcap);
      return true;
    }
  else if (type == AT_HWCAP2)
    {
      *result = GLRO(dl_hwcap2);
      return true;
    }

#ifdef HAVE_AUX_VECTOR
  for (p = GLRO(dl_auxv); p->a_type != AT_NULL; p++)
    if (p->a_type == type)
      {
        *result = p->a_un.a_val;
        return true;
      }
#endif

  return false;
}
libc_hidden_def (__getauxval2)

unsigned long int
__getauxval (unsigned long int type)
{
  unsigned long int result;

  if (__getauxval2 (type, &result))
    return result;

  __set_errno (ENOENT);
  return 0;
}

weak_alias (__getauxval, getauxval)
libc_hidden_def (__getauxval)
