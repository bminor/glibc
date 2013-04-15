#include <math.h>

#undef weak_alias
#define weak_alias(a, b)
#undef strong_alias
#define strong_alias(a, b)

#define __finite __finite_ppc32
#undef hidden_def
#define hidden_def(a) \
  __hidden_ver1 (__finite_ppc32, __GI___finite, __finite_ppc32);

#include <sysdeps/ieee754/dbl-64/s_finite.c>
