/* Set default memcmp impl based on ISA level.
   Copyright (C) 2022-2023 Free Software Foundation, Inc.
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

#include <isa-level.h>
#if MINIMUM_X86_ISA_LEVEL == 1 || MINIMUM_X86_ISA_LEVEL == 2
# define DEFAULT_MEMCMP	__memcmp_sse2
#elif MINIMUM_X86_ISA_LEVEL == 3
# define DEFAULT_MEMCMP	__memcmp_avx2_movbe
#elif MINIMUM_X86_ISA_LEVEL == 4
# define DEFAULT_MEMCMP	__memcmp_evex_movbe
#else
# error "Unknown default memcmp implementation"
#endif
