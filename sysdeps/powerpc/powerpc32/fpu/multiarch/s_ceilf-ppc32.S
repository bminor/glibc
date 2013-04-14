#include <sysdep.h>
#include <math_ldbl_opt.h>

#undef weak_alias
#define weak_alias(a,b)

#define __ceilf __ceilf_ppc32

#include <sysdeps/powerpc/powerpc32/fpu/s_ceilf.S>
