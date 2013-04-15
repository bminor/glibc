#include <math.h>

#undef weak_alias
#define weak_alias(a, b)
#undef strong_alias
#define strong_alias(a, b)

#define __isinf __isinf_ppc32
#undef hidden_def
#define hidden_def(a) \
  __hidden_ver1 (__isinf_ppc32, __GI___isinf, __isinf_ppc32);

#include <sysdeps/ieee754/dbl-64/s_isinf.c>
