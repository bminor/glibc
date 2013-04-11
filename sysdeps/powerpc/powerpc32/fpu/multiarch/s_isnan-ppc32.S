#include <sysdep.h>
#include <math_ldbl_opt.h>

#define __isnan __isnan_ppc32
#undef hidden_def
#define hidden_def(name)
  strong_alias (__isnan_ppc32, __GI___isnan)

#include <sysdeps/powerpc/powerpc32/fpu/s_isnan.S>
