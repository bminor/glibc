/* Initialize CPU feature data.  LoongArch64 version.
   This file is part of the GNU C Library.
   Copyright (C) 2022 Free Software Foundation, Inc.

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

#ifndef _CPU_FEATURES_LOONGARCH64_H
#define _CPU_FEATURES_LOONGARCH64_H

#include <sys/auxv.h>

#define SUPPORT_UAL (GLRO (dl_hwcap) & HWCAP_LOONGARCH_UAL)
#define SUPPORT_LSX (GLRO (dl_hwcap) & HWCAP_LOONGARCH_LSX)
#define SUPPORT_LASX (GLRO (dl_hwcap) & HWCAP_LOONGARCH_LASX)

#endif /* _CPU_FEATURES_LOONGARCH64_H  */

