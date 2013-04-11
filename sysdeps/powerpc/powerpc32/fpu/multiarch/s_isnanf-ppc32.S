#include <sysdep.h>
#include <math_ldbl_opt.h>

#undef weak_alias
#define weak_alias(a, b)
#undef compat_symbol
#define compat_symbol(a, b, c, d)

#define __isnan __isnanf_ppc32
#undef hidden_def
#define hidden_def(name) \
  strong_alias (__isnanf_ppc32, __GI___isnanf)

#include <sysdeps/powerpc/powerpc32/fpu/s_isnan.S>
