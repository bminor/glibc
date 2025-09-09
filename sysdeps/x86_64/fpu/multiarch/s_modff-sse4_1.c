#include <math_private.h>

#define __modff __modff_sse41
#define truncf __truncf

#include <sysdeps/ieee754/flt-32/s_modff.c>
