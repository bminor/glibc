#include <math_ldbl_opt.h>

#undef __llrint
#define __llrint __llrint_ppc32

#undef weak_alias
#define weak_alias(a, b)
#undef strong_alias
#define strong_alias(a, b)
#undef compat_symbol
#define compat_symbol(a, b, c, d)

#include <sysdeps/powerpc/powerpc32/fpu/s_llrint.c>
