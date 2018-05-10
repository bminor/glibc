/* Common definition for ifunc selections optimized with AVX2/FMA.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <init-arch.h>
#include <x86-math-features.h>

extern __typeof (REDIRECT_NAME) OPTIMIZE (sse2) attribute_hidden;
extern __typeof (REDIRECT_NAME) OPTIMIZE (fma) attribute_hidden;

static inline void *
IFUNC_SELECTOR (void)
{
  unsigned int features = __x86_math_features ();

  if ((features & x86_math_feature_fma)
      && (features & x86_math_feature_avx2))
    return OPTIMIZE (fma);

  return OPTIMIZE (sse2);
}
