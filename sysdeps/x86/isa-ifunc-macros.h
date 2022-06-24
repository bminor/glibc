/* Common ifunc selection utils
   All versions must be listed in ifunc-impl-list.c.
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

#ifndef _ISA_IFUNC_MACROS_H
#define _ISA_IFUNC_MACROS_H 1

#include <isa-level.h>
#include <sys/cdefs.h>
#include <stdlib.h>

/* Only include at the level of the minimum build ISA or higher. I.e
   if built with ISA=V1, then include all implementations. On the
   other hand if built with ISA=V3 only include V3/V4
   implementations. If there is no implementation at or above the
   minimum build ISA level, then include the highest ISA level
   implementation.  */
#if MINIMUM_X86_ISA_LEVEL <= 4
# define X86_IFUNC_IMPL_ADD_V4(...) IFUNC_IMPL_ADD (__VA_ARGS__)
#endif
#if MINIMUM_X86_ISA_LEVEL <= 3
# define X86_IFUNC_IMPL_ADD_V3(...) IFUNC_IMPL_ADD (__VA_ARGS__)
#endif
#if MINIMUM_X86_ISA_LEVEL <= 2
# define X86_IFUNC_IMPL_ADD_V2(...) IFUNC_IMPL_ADD (__VA_ARGS__)
#endif
#if MINIMUM_X86_ISA_LEVEL <= 1
# define X86_IFUNC_IMPL_ADD_V1(...) IFUNC_IMPL_ADD (__VA_ARGS__)
#endif

#ifndef X86_IFUNC_IMPL_ADD_V4
# define X86_IFUNC_IMPL_ADD_V4(...)
#endif
#ifndef X86_IFUNC_IMPL_ADD_V3
# define X86_IFUNC_IMPL_ADD_V3(...)
#endif
#ifndef X86_IFUNC_IMPL_ADD_V2
# define X86_IFUNC_IMPL_ADD_V2(...)
#endif
#ifndef X86_IFUNC_IMPL_ADD_V1
# define X86_IFUNC_IMPL_ADD_V1(...)
#endif

/* Both X86_ISA_CPU_FEATURE_USABLE_P and X86_ISA_CPU_FEATURES_ARCH_P
   macros are wrappers for the the respective
   CPU_FEATURE{S}_{USABLE|ARCH}_P runtime checks.  They differ in two
   ways.

    1.  The USABLE_P version is evaluated to true when the feature
        is enabled.

    2.  The ARCH_P version has a third argument `not`.  The `not`
        argument can either be '!' or empty.  If the feature is
        enabled above an ISA level, the third argument should be empty
        and the expression is evaluated to true when the feature is
        enabled.  If the feature is disabled above an ISA level, the
        third argument should be `!` and the expression is evaluated
        to true when the feature is disabled.
 */

#define X86_ISA_CPU_FEATURE_USABLE_P(ptr, name)                        \
  (((name##_X86_ISA_LEVEL) <= MINIMUM_X86_ISA_LEVEL)                   \
   || CPU_FEATURE_USABLE_P (ptr, name))


#define X86_ISA_CPU_FEATURES_ARCH_P(ptr, name, not)                    \
  (((name##_X86_ISA_LEVEL) <= MINIMUM_X86_ISA_LEVEL)                   \
   || not CPU_FEATURES_ARCH_P (ptr, name))


#endif
