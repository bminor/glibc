/* Linux/x86 CET initializers function.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.

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

#include <sys/prctl.h>
#include <asm/prctl.h>

static __always_inline int
dl_cet_disable_cet (unsigned int cet_feature)
{
  if (cet_feature != GNU_PROPERTY_X86_FEATURE_1_SHSTK)
    return -1;
  long long int kernel_feature = ARCH_SHSTK_SHSTK;
  return (int) INTERNAL_SYSCALL_CALL (arch_prctl, ARCH_SHSTK_DISABLE,
				      kernel_feature);
}

static __always_inline int
dl_cet_lock_cet (unsigned int cet_feature)
{
  if (cet_feature != GNU_PROPERTY_X86_FEATURE_1_SHSTK)
    return -1;
  /* Lock all SHSTK features.  */
  long long int kernel_feature = -1;
  return (int) INTERNAL_SYSCALL_CALL (arch_prctl, ARCH_SHSTK_LOCK,
				      kernel_feature);
}

static __always_inline unsigned int
dl_cet_get_cet_status (void)
{
  unsigned long long kernel_feature;
  unsigned int status = 0;
  if (INTERNAL_SYSCALL_CALL (arch_prctl, ARCH_SHSTK_STATUS,
			     &kernel_feature) == 0)
    {
      if ((kernel_feature & ARCH_SHSTK_SHSTK) != 0)
	status = GNU_PROPERTY_X86_FEATURE_1_SHSTK;
    }
  return status;
}

/* Enable shadow stack with a macro to avoid shadow stack underflow.  */
#define ENABLE_X86_CET(cet_feature)				\
  if ((cet_feature & GNU_PROPERTY_X86_FEATURE_1_SHSTK))		\
    {								\
      long long int kernel_feature = ARCH_SHSTK_SHSTK;		\
      INTERNAL_SYSCALL_CALL (arch_prctl, ARCH_SHSTK_ENABLE,	\
			     kernel_feature);			\
    }
