#include <math.h>

#undef weak_alias
#define weak_alias(a, b)

#define __logbf __logbf_ppc32

#include <sysdeps/ieee754/flt-32/s_logbf.c>
