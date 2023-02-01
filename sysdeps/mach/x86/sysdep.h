/* Copyright (C) 1991-2023 Free Software Foundation, Inc.

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

#ifndef _MACH_X86_SYSDEP_H
#define _MACH_X86_SYSDEP_H 1

/* Defines RTLD_PRIVATE_ERRNO and USE_DL_SYSINFO.  */
#include <dl-sysdep.h>
#include <tls.h>

#define LOSE asm volatile ("hlt")

#define SNARF_ARGS(entry_sp, argc, argv, envp)				      \
  do									      \
    {									      \
      char **p;								      \
      argc = (int) *entry_sp;						      \
      argv = (char **) (entry_sp + 1);					      \
      p = argv;								      \
      while (*p++ != NULL)						      \
	;								      \
      if (p >= (char **) argv[0])					      \
	--p;								      \
      envp = p;							      \
    } while (0)

#define STACK_GROWTH_DOWN

/* Get the machine-independent Mach definitions.  */
#include <sysdeps/mach/sysdep.h>

#endif /* mach/x86/sysdep.h */
