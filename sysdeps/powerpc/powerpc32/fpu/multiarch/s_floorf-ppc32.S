#include <sysdep.h>
#include <math_ldbl_opt.h>

#undef weak_alias
#define weak_alias(a,b)

#define __floorf __floorf_ppc32

#include <sysdeps/powerpc/powerpc32/fpu/s_floorf.S>
