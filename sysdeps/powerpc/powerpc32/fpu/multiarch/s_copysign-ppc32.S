#include <sysdep.h>
#include <math_ldbl_opt.h>

#undef compat_symbol
#define compat_symbol(a, b, c, d)

#define __copysign __copysign_ppc32
#undef hidden_def
#define hidden_def(name)
  strong_alias (__copysign_ppc32, __GI___copysign)

#include <sysdeps/powerpc/powerpc32/fpu/s_copysign.S>
