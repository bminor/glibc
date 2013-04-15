#include <sysdep.h>
#include <math_ldbl_opt.h>

#undef weak_alias
#define weak_alias(a,b)
#undef strong_alias
#define strong_alias(a,b)
#undef compat_symbol
#define compat_symbol(a,b,c,d)

#define __lround __lround_ppc32

#include <sysdeps/powerpc/powerpc32/fpu/s_lround.S>
