#include <math.h>

#undef weak_alias
#define weak_alias(a, b)

#define __isinff __isinff_ppc32
#undef hidden_def
#define hidden_def(a) \
  __hidden_ver1 (__isinff_ppc32, __GI___isinff, __isinff_ppc32);

#include <sysdeps/ieee754/flt-32/s_isinff.c>
