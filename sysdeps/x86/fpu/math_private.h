/* Private inline math functions for x86.
   Copyright (C) 1995-2025 Free Software Foundation, Inc.
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

#ifndef X86_MATH_PRIVATE_H
#define X86_MATH_PRIVATE_H 1

#include <math.h>
#include_next <math_private.h>

__extern_always_inline long double
__NTH (__ieee754_atan2l (long double y, long double x))
{
  long double ret;
  __asm__ __volatile__ ("fpatan" : "=t" (ret) : "0" (x), "u" (y) : "st(1)");
  return ret;
}

__extern_always_inline double
__trunc (double x)
{
#ifdef __AVX__
  asm ("vroundsd $11, %1, %1, %0" : "=v" (x) : "v" (x));
#elif defined __SSE4_1__
  asm ("roundsd $11, %1, %0" : "=x" (x) : "x" (x));
#else
  x = trunc (x);
#endif
  return x;
}

__extern_always_inline float
__truncf (float x)
{
#ifdef __AVX__
  asm ("vroundss $11, %1, %1, %0" : "=v" (x) : "v" (x));
#elif defined __SSE4_1__
  asm ("roundss $11, %1, %0" : "=x" (x) : "x" (x));
#else
  x = truncf (x);
#endif
  return x;
}

#endif
