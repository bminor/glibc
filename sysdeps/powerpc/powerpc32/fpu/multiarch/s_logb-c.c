#include <math.h>

#undef weak_alias
#define weak_alias(a, b)
#undef strong_alias
#define strong_alias(a, b)

#define __logb __logb_ppc32

#include <sysdeps/ieee754/dbl-64/s_logb.c>
